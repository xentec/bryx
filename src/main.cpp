#include "map.h"
#include "game.h"
#include "util.h"

#include <cppformat/format.h>

#include <iostream>
#include <fstream>
#include <sstream>


bool parseStartupArguments(int argc, char* argv[], std::string& map_name)
{
	if (argc < 2)
	{
		return false;
	}

	//std::cout << "argc: " << argc << std::endl;

	for (int i = 1; i < argc - 1; i++)
	{
		std::string arg = argv[i];
		std::string argn = argv[i + 1];

		if (argn.empty())
			break;

		if (arg == "-h")
		{
			//Show Help
			std::cout << "Help ...." << std::endl;
		}
		else if (arg == "-m")
		{
			//Set Map
			std::cout << "map: " << argn << std::endl;
			map_name = argn;
		}
		else if (arg == "-ip")
		{
			//Set IP
			std::cout << "ip: " << argn << std::endl;
		}
		else if (arg == "-port")
		{
			//Set Port
			std::cout << "port: " << argn << std::endl;
		}
	}

	return true;
}

int main(int argc, char* argv[])
{
	string mapFilePath = "dust.map";

	if (!parseStartupArguments(argc, argv, mapFilePath))
	{
		fmt::print("No map assigned! Please check your arguments");
		return 0;
	}


	Game game = Game::load(mapFilePath);

	fmt::print("Players: {}\n", game.players);
	fmt::print("Overrides: {}\n", game.overrides);
	fmt::print("Bombs: {} ({})\n", game.bombs, game.bombsStrength);
	fmt::print("Map: {}x{}\n", game.map->width, game.map->height);

	printMapColored(game.map);

	return 0;
}
