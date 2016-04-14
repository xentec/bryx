#include "map.h"
#include "game.h"
#include "util.h"

#include <cppformat/format.h>

#include <iostream>
#include <fstream>
#include <sstream>


template<class T>
inline
T input(Cell::Type player, string key)
{
	T var;
	fmt::print("P{} {} > ", (char) player, key);
//	std::cin >> var;
	return var;
}

int main(int argc, char* argv[])
{
	string mapFilePath = "dust.map";

	if(argc < 2)
		return 0;

	mapFilePath = argv[1];

	Game game = Game::load(mapFilePath);

	fmt::print("Players: {}\n", game.players);
	fmt::print("Overrides: {}\n", game.overrides);
	fmt::print("Bombs: {} ({})\n", game.bombs, game.bombsStrength);
	fmt::print("Map: {}x{}\n", game.map->width, game.map->height);
	game.map->print();

	u32 ply = 0;
	do
	{
		fmt::print("\n");
		fmt::print("Player {}\n", (char) game.me);
		fmt::print("################\n");

		std::vector<Move> moves = game.possibleMoves();

		u32 stoneSum = 0;
		u32 stoneMax = 0;
		std::vector<Move*> bestMoves;
		for(Move& m: moves)
		{
			int stones = m.stones.size();
			stoneSum += stones;
			if(stones > stoneMax)
			{
				bestMoves.clear();
				stoneMax = stones;
				bestMoves.push_back(&m);
				continue;
			}
			if(stones == stoneMax)
				bestMoves.push_back(&m);
		}

		if(moves.size())
		{
			fmt::print("{} moves with {} possible captures\n", (char)game.me, moves.size(), stoneMax);

			for(Move& move: moves)
			{
				fmt::print("{}>{} --> {}\n\n", move.start.pos, dir2str(move.dir), move.end->pos);
				std::unordered_set<Vec2> hl;
				hl.insert(move.start.pos);
				hl.insert(move.end->pos);

				for(Cell* c: move.stones)
					hl.insert(c->pos);

				game.map->print(hl);
			}

			fmt::print("Best moves being:\n");
			for(Move* bm: bestMoves)
				fmt::print("\t{}>{} --> {}\n", bm->start.pos, dir2str(bm->dir), bm->end->pos);
		} else
		{
			fmt::print("Cannot move", (char) game.me);
		}
/*
		Vec2 from;
		string dirStr;

		from = input<Vec2>(game.me, "Start [Vec2]"); fmt::print("\n");
		from = { 8, 2 };

		game.map->print({from});

		dirStr = input<string>(game.me, "Direction [Dir]");	fmt::print("\n");
		dirStr = "W";


		Direction dir = str2dir(dirStr);
		Move move { game.map->at(from), dir };

		Move::Error res = game.testMove(move);
		fmt::print("Errors? {}\n", Move::err2str(res));

		if(res != Move::Error::NONE) return 0;

		for(Cell* c: move.stones)
			c->type = game.me;
*/
		game.me = (Cell::Type)((u32)Cell::Type::P1 + ++ply % game.players);
		if(ply == game.players)
			break;
	}
	while(true);

	fmt::print("\n################\n");
	fmt::print("GAME SET\n");
	fmt::print("################\n\n");
	game.map->print();

	return 0;
}
