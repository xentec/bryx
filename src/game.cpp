#include "game.h"

#include "util.h"

#include <cppformat/format.h>
#include <fstream>

Game::~Game()
{
	delete map;
}

bool Game::isValidMove(Move& move)
{
	return true;
}

Game Game::load(std::string filename)
{
	using std::stoi;

	std::ifstream file(filename);
	if (!file.good())
		throw std::runtime_error(fmt::format("File '{}' cannot be read", filename));

	Game game;
	game.players = stoi(readline(file));
	game.overrides = stoi(readline(file));

	std::vector<string> s;

	// Bombs
	s = splitString(readline(file), ' ');
	if(s.size() < 2)
		throw std::runtime_error("Failed to parse map: bombs delimiter not found");

	game.bombs = stoi(s[0]);
	game.bombsStrength = stoi(s[1]);

	// Map size
	s = splitString(readline(file), ' ');
	if(s.size() < 2)
		throw std::runtime_error("Failed to parse map: map dimensions delimiter not found");

	game.map = new Map(stoi(s[1]), stoi(s[0]));

	for (i32 y = 0; y < game.map->height; y++)
	{
		std::string line = readline(file);
		usz lm = (line.length()+1)/2;

		if(lm != game.map->width)
			throw std::runtime_error(fmt::format("Failed to parse map: length of line {} does not match map width: {} != {}", y+5, lm, game.map->width));

		for(i32 x = 0; x < line.length(); x += 2) // ignore spaces inbetween
		{
			char c = line[x];
			if(!Cell::isValid(c))
				throw std::runtime_error(fmt::format("Failed to parse map: invalid cell character '{}' found at {}:{}", c, y+5, x));

			game.map->at(x/2, y).type = static_cast<Cell::Type>(c);
		}
	}

	string line;
	while(readline(file, line))
	{
		if(line.empty()) continue;

		// Parse Transistion
		s = splitString(line, ' ');

		Cell& from = game.map->at({ stoi(s.at(0)), stoi(s.at(1)) }),
				to = game.map->at({ stoi(s.at(4)), stoi(s.at(5)) });

		try {
			from.addTransistion(static_cast<Direction>(stoi(s.at(2))), &to);
			to.addTransistion(static_cast<Direction>(stoi(s.at(6))), &from);
		}
		catch(std::out_of_range& e)
		{
			fmt::print("Failed to set transistion [{} <-> {}]: position is outside the map [{}]\n", from.asString(), to.asString(), e.what());
		}
		catch(std::exception& e)
		{
			fmt::print("Failed to set transistion [{} <-> {}]: {}\n", from.asString(), to.asString(), e.what());
		}
	}
	return game;
}
