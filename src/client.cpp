#include "client.h"

#include "game.h"
#include "player.h"
#include "vector.h"

#include <cstdio>
#include <sstream>

struct Dummy : Player
{
	Dummy(Game& game, Cell::Type color): Player(game, color) {}
	Dummy(const Dummy& other): Player(other) {}
	virtual ~Dummy() {}
	virtual Player* clone() const { return new Dummy(*this); }
	virtual Move move(std::deque<Move>&, u32, u32) { return Move(*this, nullptr); }
};



Client::Client()
{}

void Client::join(string host, u16 port)
{
	try
	{
		println("Connecting to {}:{} ...", host, port);

		socket.connect(host, port);
		socket.send(packet::Join(YIMB_GROUP).dump());
	}
	catch(const std::exception& ex)
	{
		throw std::runtime_error(fmt::format("failed to connect: {}", ex.what()));
	}

	try
	{
		println("Waiting for Map...");
		std::stringstream mapData(read<packet::Map>().map);
		game.load(mapData);
	}
	catch(const std::exception& ex)
	{
		throw std::runtime_error(fmt::format("failed to load map: {}", ex.what()));
	}

	u32 plyID = read<packet::PlayerID>().player - 1;

	for(u32 i = 0; i < game.defaults.players; i++)
	{
		if(plyID == i)
			me = &game.addPlayer<AI>();
		else
			game.addPlayer<Dummy>();
	}

	print("\n");
	println("My number: {}", *me);
	print("\n");

	println("Players: {}", game.defaults.players);
	println("Overrides: {}", game.defaults.overrides);
	println("Bombs: {} ({})", game.defaults.bombs, game.defaults.bombsStrength);
	println("Map: {}x{}", game.getMap().width, game.getMap().height);
	game.getMap().print();
	print("\n");

	println("Avg Eval time: {} ms", game.aiData.evalTime.count());
}

void Client::play()
{
	while(true)
	{
		switch(peek().type)
		{
		case packet::MOVE_REQUEST:
			{
				auto packet = read<packet::MoveRequest>(); // TODO: take time and depth into account

				println("Got move request: time {}, depth {}", packet.time, packet.depth);

				game.currPly = type2ply(me->color);

#if MOVES_ITERATOR
				std::deque<Move> moves = me->possibleMoves().all();
#else
				std::deque<Move> moves = me->possibleMoves();
#endif
				if(moves.empty())
					throw std::runtime_error("no moves found");

				Move move = me->move(moves, packet.time, packet.depth);
				packet::MoveResponse resp(move.target->pos, move.bonus);

				if(move.choice != Cell::Type::VOID)
					resp.extra = type2ply(move.choice)+1;

				println("Sending move: {} -> {} ex: {}", move.player, move.target->pos, resp.extra);
				send(resp);

				move.print();
			}
			break;
		case packet::MOVE:
			{
				auto packet = read<packet::Move>();

				game.currPly = packet.player - 1;
				Player& ply = game.currPlayer();

				println("Got player move: {} -> {} ex: {}", ply, vec{packet.x, packet.y}, packet.extra);

				Move move = { ply, nullptr };
				move.target = &game.getMap().at(packet.x, packet.y);

				ply.evaluate(move);

				if(packet.extra)
				{
					if(packet.extra == Move::Bonus::BOMB || packet.extra == Move::Bonus::OVERRIDE)
					{
						 move.bonus = static_cast<Move::Bonus>(packet.extra);
					} else
						move.choice = ply2type(packet.extra-1);
				}

				if(ply.color != me->color)
				{
					move.print();
					print("\n\n");
				}
				game.execute(move);
			}
			break;
		case packet::DISQ:
		{
			auto packet = read<packet::Disq>();

			auto dp = game.getPlayers().begin() + packet.player - 1;
			if(*dp == me) // NOOOOOOOOOOOO!!!!!111111
			{
				println("It was nice while it lasted... ;_;");
				return;
			}
			delete *dp;
			game.getPlayers().erase(dp);
		}
			break;
		case packet::BOMB_PHASE:
		{
			read<packet::BombPhase>(); // clear socket buffer
			// TODO: Change phase

			println("Switching to BOMB phase!");
		}
			break;
		case packet::GAME_END:
		{
			read<packet::GameEnd>(); // clear socket buffer
			println("gg kthxbye");
			return;
		}
		default:
			throw std::runtime_error(fmt::format("unknown packet received: {}", peek().type));
		}
	}
}

packet::Header Client::peek() const
{
	return packet::Header(socket.recv(5, true));
}
