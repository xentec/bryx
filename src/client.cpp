#include "client.h"

#include "game.h"
#include "player.h"
#include "vector.h"

#include <cstdio>
#include <sstream>

struct Dummy : Player
{
	Dummy(Game& game, Cell::Type color): Player(game, color) {}
	virtual ~Dummy() {}
	Move move(u32, u32) { return Move(*this, nullptr); }
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

	auto plyID = read<packet::PlayerID>();

	for(u32 i = 0; i < game.defaults.players; i++)
	{
		if(plyID.player - 1 == i)
			me = &game.addPlayer<AI>();
		else
			game.addPlayer<Dummy>();
	}

	fmt::print("\n");
	fmt::print("My number: {}\n", me->color);
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

				Move move = me->move(packet.time, packet.depth);
				send(packet::MoveResponse(move.target->pos, move.bonus));
			}
			break;
		case packet::MOVE:
			{
				auto packet = read<packet::Move>();

				fmt::print("Got player move: P{} -> {} ex: {}\n", packet.player, vec{packet.x, packet.y}, packet.extra);

				Move move = game.getPlayers()[packet.player-1]->move(0,0);
				move.target = &game.getMap().at(packet.x, packet.y);
				move.bonus = static_cast<Move::Bonus>(packet.extra);

				game.execute(move);
			}
			break;
		case packet::DISQ:
		{
			auto packet = read<packet::Disq>();

			usz pos = packet.player - 1;

			auto dp = game.getPlayers().begin()+pos;
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
