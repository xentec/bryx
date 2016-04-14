#include "game.h"

#include "util.h"

#include <cppformat/format.h>
#include <fstream>


Game::Game()
{}

Game::~Game()
{
	delete map;
}

std::vector<Move> Game::possibleMoves() const
{
	std::vector<Move> moves;

	for(i32 x = 0; x < map->width; x++)
	for(i32 y = 0; y < map->height; y++)
	{
		auto cellMoves = possibleMovesOn(map->at(x, y));
		moves.insert(moves.end(), cellMoves.begin(), cellMoves.end());
	}

	return moves;
}

std::vector<Move> Game::possibleMovesOn(Cell& cell) const
{
	std::vector<Move> moves;

	for(u8 dir = Direction::N; dir < Direction::LAST; dir++)
	{
		Move move { cell, (Direction) dir };
		if(testMove(move) == Move::Error::NONE)
			moves.push_back(move);
	}

	return moves;
}

Move::Error Game::testMove(Move& move) const
{
	move.stones.clear();

	if(move.start.type != me)
		return Move::Error::WRONG_START;

	Direction moveDir = move.dir;

	Cell* cur = move.start.getNeighbor(moveDir);
	while(cur && cur->isCaptureable())
	{
		move.end = cur;
		if(cur->type == me)
			return Move::Error::PATH_BLOCKED;

		move.stones.push_back(cur);
#if DEBUG
		fmt::print("\nNEW STONE: {}\n", cur->asString());
		map->print({cur->pos});
#endif

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

	move.end = (cur) ?: move.stones.back();

	if(!cur || !cur->isFree())
			return Move::Error::LINE_FULL;

	return Move::Error::NONE;
}

Game Game::load(std::istream& file)
{
	using std::stoi;

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


Move::Move(Cell& start, Direction dir):
	start(start), end(nullptr), dir(dir)
{}

Move::Move(const Move& other):
	start(other.start), end(other.end), dir(other.dir),
	err(other.err),
	stones(other.stones)
{}

Move&Move::operator =(const Move& other)
{
	start = other.start;
	dir = other.dir;
	end = other.end;
	err = other.err;
	stones = other.stones;
	return *this;
}

std::string Move::err2str(Move::Error err)
{
	switch(err)
	{
	case Error::NO_STONES_CAPTURED: return "no stones to capture";
	case Error::LINE_FULL: return "no free field in move line found";
	case Error::PATH_BLOCKED: return "path is not correct";
	case Error::WRONG_START: return "start field is not in possesion";
	default:
		return "none";
	}
}
