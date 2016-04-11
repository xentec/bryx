#include "game.h"

#include "util.h"

#include <cppformat/format.h>
#include <fstream>

static bool vec2dir(const Vec2& v, Direction& dir)
{
	if(v.x != 0 && v.y != 0 && v.x != v.y)
		return false;

	Direction dirH;
	switch(v.x / std::abs(v.x))
	{
	case -1: dirH = Direction::W; break;
	case 0: break;
	case 1: dirH = Direction::E; break;
	}

	Direction dirV;
	switch(v.y / std::abs(v.y))
	{
	case -1: dirV = Direction::N; break;
	case 0: break;
	case 1: dirV = Direction::S; break;
	}

	return true;
}

Cell* Game::test = nullptr;

Game::Game()
{}

Game::~Game()
{
	delete map;
}

Move::Error Game::testMove(Move& move)
{
	move.stones.clear();

	if(move.from.type != me)
		return Move::Error::WRONG_START;

	Direction moveDir = move.dir;

	Cell* cur = move.from.getNeighbor(moveDir);
	while(cur && cur->isCaptureable())
	{
		if(cur->type == me)
			return Move::Error::WRONG_PATH;



		move.stones.push_back(cur);
//#if DEBUG
		fmt::print("\nNEW STONE: {}\n", cur->asString());
		map->print(&cur->pos);
//#endif

		Cell::Transition tr = cur->transitions[(usz) moveDir];
		if(tr.target)
		{
			cur = tr.target;
			moveDir = tr.out;
		} else
			cur = cur->getNeighbor(moveDir);
	}
	if(move.stones.empty())
		return Move::Error::NO_STONES_CAPTURED;

	return Move::Error::NONE;
}

Game Game::load(std::string filename)
{
	using std::stoi;

	std::ifstream file(filename);
	if (!file)
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

		Cell &from = game.map->at(stoi(s.at(0)), stoi(s.at(1))),
			   &to = game.map->at(stoi(s.at(4)), stoi(s.at(5)));

		Direction in = static_cast<Direction>(stoi(s.at(2))),
				 out = static_cast<Direction>(stoi(s.at(6)));

		try {
			from.addTransistion(in, out, &to);
			to.addTransistion(out, in, &from);

			Game::test = &from;
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


std::string Move::err2str(Move::Error err)
{
	switch(err)
	{
	case Error::NO_STONES_CAPTURED: return "no stones to capture";
	case Error::TARGET_OCCUPIED: return "target field is occupied";
	case Error::WRONG_PATH: return "path is not correct";
	case Error::WRONG_START: return "start field is not in possesion";
	default:
		return "none";
	}
}
