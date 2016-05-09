#include "game.h"

#include "map.h"
#include "util.h"

#include <cppformat/format.h>

#include <list>

#define VERBOSE 1

Game::Game():
	defaults{ 0, 0, 0, 0 }, stats { 0, 0, 0 },
	map(nullptr),
	currentPlayer(0), moveless(0)
{}

Game::~Game()
{
	for(Player* ply: players)
		delete ply;
}

Player& Game::addPlayer(Player* player)
{
	player->id = players.size();
	player->color = (Cell::Type) (Cell::Type::P1 + player->id);
	player->bombs = defaults.bombs;
	player->overrides = defaults.overrides;
	players.push_back(player);
	return *players.back();
}

Player& Game::nextPlayer()
{
	return *players[currentPlayer++ % players.size()];
}

Map& Game::getMap() const
{
	return *map;
}

std::vector<Player*>& Game::getPlayers()
{
	return players;
}

bool Game::hasEnded()
{
	return moveless == players.size();
}

void Game::run()
{

	std::chrono::time_point<std::chrono::system_clock> start, end;
	std::chrono::duration<double> elapsed;
	u32 num = 1;

	do
	{
		Player& ply = nextPlayer();
#if VERBOSE
		fmt::print("\n");
		fmt::print("Player {} {}\n", ply.id+1, ply.name);

		fmt::print("##########");
		for(usz i = 0; i < ply.name.size(); i++)
			fmt::print("#");
		fmt::print("\n");

		fmt::print("Overrides: {}\n", ply.overrides);
		fmt::print("Bombs: {}\n", ply.bombs);
		fmt::print("\n");
		fmt::print("State scores\n");
		for(Player* p: players)
			fmt::print("Player {}: {}\n", p->id+1, p->score());
#endif
		start = std::chrono::system_clock::now();
		Move move = ply.move();
		end = std::chrono::system_clock::now();

		elapsed = end-start;
		if(elapsed > stats.time.moveMax)
			stats.time.moveMax = elapsed;

		stats.time.moveAvg += elapsed/(num*(num+1));
		num++;

		if(!move.target)
		{
#if VERBOSE
			fmt::print("No move\n");
#endif
			moveless++;
			continue;
		}

		execute(move);

#if VERBOSE
		fmt::print("\n  \n\n", move.asString());
		move.print();
#endif
	}
	while(!hasEnded());
}

void Game::evaluate(Move& move) const
{
	if(move.target->type == Cell::Type::VOID)
	{
		move.err = Move::Error::WRONG_START;
		return;
	}

	if(move.target->isCaptureable())
		move.override = true;

	Direction banned = Direction::_LAST;

	for(u32 dir = Direction::N; dir < Direction::_LAST; dir++)
	{
		Direction moveDir = (Direction) dir;
		if(dir == banned)
			continue;

		Cell* cp = move.target;

		std::list<Cell*> line;
		Cell* cur = move.target->getNeighbor(moveDir);
		while(cur && cur->isCaptureable())
		{
			if(cur->type == move.player.color)
			{
				if(!line.empty())
					move.captures.push_back(line);
				break;
			}

			if(cp == cur) // we're in a loop!
			{
				 // do not try the same dir you came from while looping
				banned = dir180(moveDir);
				break;
			}

			line.push_back(cur);
			cur = cur->getNeighbor(moveDir);
		}
	}

	if(move.captures.empty())
		move.err =  Move::Error::NO_CONNECTIONS;
	return;
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
		stats.overrides++;
	}

	Cell::Type targetCell = move.target->type; // first make a complete move...

	move.target->type = move.player.color;

	for(auto& line: move.captures)
	for(Cell* c: line)
		c->type = move.player.color;


	switch(targetCell) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		move.player.bonus();
		break;
	case Cell::Type::CHOICE:
	{
		Player& swap = move.player.choice();
		if(move.player.color == swap.color)
			break;

		std::swap(swap.color, move.player.color);
	}
		break;
	case Cell::Type::INVERSION:
		for(Player* ply: players)
			ply->id = (ply->id +1) % players.size();

		stats.inversions++;
		break;
	default:
		break;
	}
	stats.moves++;
}

void Game::load(std::istream& file)
{
	using std::stoi;

	defaults.players = stoi(readline(file));
	defaults.overrides = stoi(readline(file));

	std::vector<string> tmp;

	// Bombs
	tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("bombs delimiter not found"); // TODO: better error messages placement

	defaults.bombs = stoi(tmp[0]);
	defaults.bombsStrength = stoi(tmp[1]);

	// Map size
	tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("map dimensions delimiter not found");

	map = new Map(stoi(tmp[1]), stoi(tmp[0]));
	Map& map = getMap(); // shadowing on purpose

	for (i32 y = 0; y < map.height; y++)
	{
		std::string line = readline(file);
		usz lm = (line.length()+1)/2;

		if(lm != map.width)
			throw std::runtime_error(fmt::format("length of line {} does not match map width: {} != {}", y+5, lm, map.width));

		for(i32 x = 0; x < line.length(); x += 2) // ignore spaces inbetween
		{
			char c = line[x];
			if(!Cell::isValid(c))
				throw std::runtime_error(fmt::format("invalid cell character '{}' found at {}:{}", c, y+5, x));

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

		Direction exit = static_cast<Direction>(stoi(tmp.at(2))),
				 entry = static_cast<Direction>(stoi(tmp.at(6)));

		try {
			from.addTransistion(exit, dir180(entry), &to);
			to.addTransistion(entry, dir180(exit), &from);
		}
		catch(std::out_of_range& e)
		{
			fmt::print("error in transistion [{} <-> {}]: position is outside the map [{}]\n", from.asString(), to.asString(), e.what());
		}
		catch(std::exception& e)
		{
			fmt::print("error in transistion [{} <-> {}]: {}\n", from.asString(), to.asString(), e.what());
		}
	}
}
