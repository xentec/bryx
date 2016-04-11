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

	u32 ply = 0;
//	do
	{
		game.map->print();
		Vec2 from;
		string dirStr;

		from = input<Vec2>(game.me, "Start [Vec2]"); fmt::print("\n");
		from = { 8, 2 };

		game.map->print(&from);

		dirStr = input<string>(game.me, "Direction [Dir]");	fmt::print("\n");
		dirStr = "W";


		Direction dir = str2dir(dirStr);
		Move move { game.map->at(from), dir };

		Move::Error res = game.testMove(move);
		fmt::print("Errors? {}\n", Move::err2str(res));

		if(res != Move::Error::NONE) return 0;

		for(Cell* c: move.stones)
			c->type = game.me;

		game.me = (Cell::Type)((u32)Cell::Type::P1 + ++ply % game.players);

	}
//	while(true);

	game.map->print();

	return 0;
}
