#include "game.h"

#include "util.h"

#include <cppformat/format.h>
#include <fstream>


Game::Game():
	currentPlayer(0), moveless(0)
{}

Game::~Game()
{
	delete map;
}

Player& Game::nextPlayer()
{
	currentPlayer = (currentPlayer+1) % players.size();
	return players[currentPlayer];
}

bool Game::hasEnded()
{
	return moveless == players.size();
}

Move::Error Game::testMove(Move& move) const
{
	move.stones.clear();

	if(move.start.type != move.player.color)
		return Move::Error::WRONG_START;

	Direction moveDir = move.dir;

	Cell* cur = move.start.getNeighbor(moveDir);
	while(cur && cur->isCaptureable())
	{
		move.end = cur;
		if(cur->type == move.player.color)
			return Move::Error::PATH_BLOCKED;

		move.stones.push_back(cur);

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

void Game::execute(Move &move)
{
	moveless = 0;

	for(Cell* c: move.stones)
		c->type = move.player.color;

	if(move.override)
	{
		if(overrides == 0)
			throw std::runtime_error("no overrides left");
		move.player.overrides--;
	}

	Cell::Type lastCell = move.end->type; // first make a complete move...
	move.end->type = move.player.color;

	switch(lastCell) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		move.player.bonus();
		break;
	case Cell::Type::CHOICE:
	{
		Player& swap = move.player.choice();
		if(move.player.color == swap.color)
			break;

		for(i32 x = 0; x < map->width; x++)
		for(i32 y = 0; y < map->height; y++)
		{
			Cell& c = map->at(x,y);

			if(c.type == move.player.color)
				c.type = swap.color;
			else if(c.type == swap.color)
				c.type = move.player.color;
		}
		Cell::Type tmp = swap.color;
		swap.color = move.player.color;
		move.player.color = tmp;
	}
		break;
	case Cell::Type::INVERSION:
		for(i32 x = 0; x < map->width; x++)
		for(i32 y = 0; y < map->height; y++)
		{
			Cell& c = map->at(x,y);
			if(c.isPlayer())
				c.type = (Cell::Type) ((c.type - Cell::Type::P1+1) % players.size() + Cell::Type::P1);
		}

		for(Player& ply: players)
			ply.color = (Cell::Type) ((ply.color - Cell::Type::P1+1) % players.size() + Cell::Type::P1);

		break;
	default:
		break;
	}
}

void Game::pass()
{
	moveless++;
}

Game Game::load(std::istream& file)
{
	using std::stoi;

	Game game;

	game.players.assign(stoi(readline(file)), Player(game, Cell::Type::VOID));
	for(u32 i = 0; i < game.players.size(); i++)
		game.players[i].color = (Cell::Type) (Cell::Type::P1 + i);

	game.overrides = stoi(readline(file));

	std::vector<string> tmp;

	// Bombs
	tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("Failed to parse map: bombs delimiter not found"); // TODO: better error messages placement

	game.bombs = stoi(tmp[0]);
	game.bombsStrength = stoi(tmp[1]);

	// Map size
	tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("Failed to parse map: map dimensions delimiter not found");

	game.map = new Map(stoi(tmp[1]), stoi(tmp[0]));

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
		tmp = splitString(line, ' ');

		Cell &from = game.map->at(stoi(tmp.at(0)), stoi(tmp.at(1))),
			   &to = game.map->at(stoi(tmp.at(4)), stoi(tmp.at(5)));

		Direction in = static_cast<Direction>(stoi(tmp.at(2))),
				 out = static_cast<Direction>(stoi(tmp.at(6)));

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
