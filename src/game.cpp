#include "game.h"

#include "map.h"
#include "util.h"

#include <cppformat/format.h>
#include <fstream>

#include <list>

Game::Game(Map map):
	map(map),
	defaults{ 0, 0, 0, 0 }, stats { 0, 0, 0 },
	currentPlayer(0), moveless(0)
{}

Game::~Game()
{
	for(Player* ply: players)
		delete ply;
	delete map;
}

void Game::addPlayer(Player* player)
{
	player->game = this;
	player->id = players.size();
	player->bombs = defaults.bombs;
	player->overrides = defaults.overrides;

	players.push_back(player);
}

Player& Game::nextPlayer()
{
	return *players[currentPlayer++ % players.size()];
}

bool Game::hasEnded()
{
	return moveless == players.size();
}

void Game::run()
{
	do
	{
		Move move = nextPlayer().move();
		execute(move);
	} while(!hasEnded());
}

Move::Error Game::testMove(Move& move) const
{
	if(move.target->type == Cell::Type::VOID || (move.target->isCaptureable() && !move.override))
		return Move::Error::WRONG_START;

	Direction banned = Direction::LAST;

	for(u32 dir = Direction::N; dir < Direction::LAST; dir++)
	{
		Direction moveDir = (Direction) dir;
		if(dir == banned)
			continue;

		Cell* cp = move.target;

		std::list<Cell*> line;
		Cell* cur = move.target->getNeighbor(moveDir);
		while(cur && cur->isCaptureable())
		{
			if(cp == cur) // we're in a loop!
			{
				 // do not try the same dir you came from while looping
				banned = (Direction) ((moveDir+4) % (Direction::LAST-1));
				break;
			}

			if(cur->isPlayer(move.player.id))
			{
				if(!line.empty())
					move.captures.push_back(line);
				break;
			}
			line.push_back(cur);
			cur = cur->getNeighbor(moveDir);
		}
	}

	return move.captures.empty() ? Move::Error::NO_CONNECTIONS : Move::Error::NONE;
}

void Game::execute(Move &move)
{
	if(move.captures.empty())
	{
		moveless++;
		return;
	} else
		moveless = 0;

	if(move.override)
	{
		if(move.player.overrides == 0)
			throw std::runtime_error("no overrides left");
		move.player.overrides--;
	}

	Cell::Type targetCell = move.target->type; // first make a complete move...

	move.target->setPlayer(move.player.id);

	for(auto& line: move.captures)
	for(Cell* c: line)
		c->setPlayer(move.player.id);


	switch(targetCell) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		move.player.bonus();
		break;
	case Cell::Type::CHOICE:
	{
		Player& swap = move.player.choice();
		if(move.player.id == swap.id)
			break;

		for(Cell& c: map)
		{
			if(c.isPlayer(move.player.id))
				c.setPlayer(swap.id);
			else if(c.isPlayer(swap.id))
				c.setPlayer(move.player.id);
		}
		u32 tmp = swap.id;
		swap.id = move.player.id;
		move.player.id = tmp;
	}
		break;
	case Cell::Type::INVERSION:
		for(Cell& c: map)
		{
			if(c.isPlayer())
				c.setPlayer((c.type - Cell::Type::P1 + 1) % players.size());
		}

		for(Player* ply: players)
			ply->id = (ply->id +1) % players.size();

		break;
	default:
		break;
	}
	stats.moves++;
}

void Game::pass()
{
	moveless++;
}

Game Game::load(std::istream& file)
{
	using std::stoi;

	Game::Defaults defaults;

	defaults.players = stoi(readline(file));
	defaults.overrides = stoi(readline(file));

	std::vector<string> tmp;

	// Bombs
	tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("Failed to parse map: bombs delimiter not found"); // TODO: better error messages placement

	defaults.bombs = stoi(tmp[0]);
	defaults.bombsStrength = stoi(tmp[1]);

	// Map size
	tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("Failed to parse map: map dimensions delimiter not found");

	Map map(stoi(tmp[1]), stoi(tmp[0]));

	for (i32 y = 0; y < map.height; y++)
	{
		std::string line = readline(file);
		usz lm = (line.length()+1)/2;

		if(lm != map.width)
			throw std::runtime_error(fmt::format("Failed to parse map: length of line {} does not match map width: {} != {}", y+5, lm, map.width));

		for(i32 x = 0; x < line.length(); x += 2) // ignore spaces inbetween
		{
			char c = line[x];
			if(!Cell::isValid(c))
				throw std::runtime_error(fmt::format("Failed to parse map: invalid cell character '{}' found at {}:{}", c, y+5, x));

			map.at(x/2, y).type = static_cast<Cell::Type>(c);
		}
	}

	string line;
	while(readline(file, line))
	{
		if(line.empty()) continue;

		// Parse Transistion
		tmp = splitString(line, ' ');

		Cell &from = map.at(stoi(tmp.at(0)), stoi(tmp.at(1))),
			   &to = map.at(stoi(tmp.at(4)), stoi(tmp.at(5)));

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

	Game game(std::move(map));
	game.defaults = defaults;
	return game;
}
