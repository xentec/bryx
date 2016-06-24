#include "client.h"

#include "game.h"
#include "player.h"
#include "human.h"
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
	virtual Move bomb(u32) { return Move(*this, nullptr); }
};



Client::Client(Game& game):
	game(game)
{}

void Client::join(string host, u16 port)
{
	try
	{
		println("Connecting to {}:{} ...", host, port);

		socket.connect(host, port);
		send(packet::Join(YIMB_GROUP));
	}
	catch(const std::exception& ex)
	{
		throw std::runtime_error(fmt::format("failed to connect: {}", ex.what()));
	}

	try
	{
		println("Waiting for Map...");

		packet::Header hdr;
		socket.recv(reinterpret_cast<u8*>(&hdr), 5);

		packet::Map exp;
		if(exp.hdr.type != hdr.type)
			throw std::runtime_error(fmt::format("wrong packet! expected: {}, got {}", exp.hdr.type, hdr.type));

		string mapData(hdr.size(), ' ');

		socket.recv(reinterpret_cast<u8*>(&mapData[0]), hdr.size());

		std::stringstream stream(mapData);
		game.load(stream);
	}
	catch(const std::exception& ex)
	{
		throw std::runtime_error(fmt::format("failed to load map: {}", ex.what()));
	}

	u32 plyID = recv<packet::PlayerID>().player - 1;

	for(u32 i = 0; i < game.defaults.players; i++)
	{
		if(plyID == i)
			me = (AI*) &game.addPlayer<AI>();
		else
			game.addPlayer<Dummy>();
	}

	println();
	println("My number: {}\n", *me);
}

void Client::play()
{
	while(true)
	{
		switch(peek().type)
		{
		case packet::MOVE_REQUEST:
			{
				auto packet = recv<packet::MoveRequest>(); // TODO: take time and depth into account

				println("Got move request: time {}, depth {}", packet.time(), packet.depth);

				game.currPly = type2ply(me->color);

				Move move = { *me, nullptr };

				if(game.phase == Game::Phase::REVERSI)
				{
#if MOVES_ITERATOR
					PossibleMoves moves = me->possibleMoves().all();
#else
					PossibleMoves moves = me->possibleMoves();
#endif

#if SAFE_GUARDS
					if(moves.empty())
						throw std::runtime_error("no moves found");
#endif
					move = me->move(moves, packet.time(), packet.depth);
				} else
				{
					move = me->bomb(packet.time());
				}

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
				auto packet = recv<packet::Move>();

				game.currPly = packet.player - 1;
				Player& ply = game.currPlayer();

				println("Got player move: {} -> {} ex: {}", ply, packet.pos(), packet.extra);

				Move move = { ply, &game.getMap().at(packet.pos()) };

				if(game.phase == Game::Phase::REVERSI)
				{
					ply.evaluate(move);

					if(packet.extra)
					{
						if(packet.extra == Move::Bonus::BOMB || packet.extra == Move::Bonus::OVERRIDE)
						{
							 move.bonus = static_cast<Move::Bonus>(packet.extra);
						} else
							move.choice = ply2type(packet.extra-1);
					}

					bool exp = me->playerMoved(move);

					if(ply.color != me->color)
					{
						move.print();
						println("{}", exp ? "ANTICIPATED" : "SURPRISED!");
					}
				}

				game.execute(move);
				game.getMap().print();
				print("\n\n");

			}
			break;
		case packet::DISQ:
		{
			auto packet = recv<packet::Disq>();

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
			recv<packet::BombPhase>(); // clear socket buffer

			println("Switching to BOMB phase!");
			game.phase = Game::Phase::BOMB;
		}
			break;
		case packet::GAME_END:
		{
			recv<packet::GameEnd>(); // clear socket buffer
			println("gg kthxbye");
			return;
		}
		default:
			throw std::runtime_error(fmt::format("unknown packet received: {}", peek().type));
		}

		println("Waiting for packets...");
	}
}

packet::Header Client::peek() const
{
	packet::Header hdr;
	socket.recv(reinterpret_cast<u8*>(&hdr), 5, true);
	return hdr;
}
