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
	virtual Move move(u32, u32) { return Move(*this, nullptr); }
};



Client::Client()
{}

void Client::join(string host, u16 port)
{
	try
	{
		fmt::print("Connecting to {}:{} ...\n", host, port);
		std::fflush(stdout);

		socket.connect(host, port);
		socket.send(packet::Join(YIMB_GROUP).dump());
	}
	catch(const std::exception& ex)
	{
		throw std::runtime_error(fmt::format("failed to connect: {}", ex.what()));
	}

	try
	{
		fmt::print("Waiting for Map...\n");
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

	fmt::print("\n");
	fmt::print("My number: {}\n", type2ply(me->color)); // TODO: ply-asString()
	fmt::print("\n");

	fmt::print("Players: {}\n", game.defaults.players);
	fmt::print("Overrides: {}\n", game.defaults.overrides);
	fmt::print("Bombs: {} ({})\n", game.defaults.bombs, game.defaults.bombsStrength);
	fmt::print("Map: {}x{}\n", game.getMap().width, game.getMap().height);
	game.getMap().print();


	while(true)
	{
		switch(peek().type)
		{
		case packet::MOVE_REQUEST:
			{
				auto packet = read<packet::MoveRequest>(); // TODO: take time and depth into account

				fmt::print("Got move request: time {}, depth {}\n", packet.time, packet.depth);

				game.currPly = type2ply(me->color);
				Move move = me->move(packet.time, packet.depth);
				packet::MoveResponse resp(move.target->pos, move.bonus);

				if(move.choice != Cell::Type::VOID)
					resp.extra = type2ply(move.choice)+1;

				fmt::print("Sending move: {} -> {} ex: {}\n", move.player, move.target->pos, resp.extra);
				move.print();

				send(resp);
			}
			break;
		case packet::MOVE:
			{
				auto packet = read<packet::Move>();

				game.currPly = packet.player - 1;
				Player& ply = game.currPlayer();

				fmt::print("Got player move: {} -> {} ex: {}\n", ply, vec{packet.x, packet.y}, packet.extra);

				Move move = ply.move(0,0);
				move.target = &game.getMap().at(packet.x, packet.y);

				ply.evaluate(move);

				if(packet.extra)
				{
					if(packet.extra == 20 || packet.extra == 21)
					{
						 move.bonus = static_cast<Move::Bonus>(packet.extra);
					} else
						move.choice = ply2type(packet.extra-1);
				}

				game.execute(move);
				move.print();
			}
			break;
		case packet::DISQ:
		{
			auto packet = read<packet::Disq>();

			auto dp = game.getPlayers().begin() + packet.player - 1;
			if(*dp == me) // NOOOOOOOOOOOO!!!!!111111
			{
				fmt::print("It was nice while it lasted... ;_;\n");
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
		}
			break;
		case packet::GAME_END:
		{
			read<packet::GameEnd>(); // clear socket buffer
			fmt::print("gg kthxbye\n");
			return;
		}
			break;
		default:
			break;
		}
	}
}

packet::Header Client::peek() const
{
	return packet::Header(socket.recv(5, true));
}
