#include "game.h"

#include "map.h"
#include "util.h"

#include <fmt/format.h>

#include <list>

#define VERBOSE 1

Game::Game():
	defaults{ 0, 0, 0, 0 }, stats { 0, 0, 0, {} },
	map(nullptr),
	currentPlayer(0), moveless(0)
{}

Game::Game(const Game &other):
	defaults(other.defaults), stats(other.stats),
	map(nullptr),
	currentPlayer(other.currentPlayer), moveless(other.moveless)
{
	if(other.map)
		map = new Map(*other.map);

	for(Player* p: other.players)
		players.push_back(p->clone());
}

Game::~Game()
{
	for(Player* ply: players)
		delete ply;

	delete map;
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
	if(players.size() < defaults.players)
		throw std::runtime_error("not enough players");

	std::chrono::time_point<std::chrono::system_clock> start, end;
	std::chrono::duration<double> elapsed;
	u32 num = 1;

	do
	{
		Player& ply = nextPlayer();
#if VERBOSE
		fmt::print("\n");
		fmt::print("Player {} {}\n", type2ply(ply.color), ply.name);

		fmt::print("##########");
		for(usz i = 0; i < ply.name.size(); i++)
			fmt::print("#");
		fmt::print("\n");

		fmt::print("Overrides: {}\n", ply.overrides);
		fmt::print("Bombs: {}\n", ply.bombs);
		fmt::print("\n");
#endif
		start = std::chrono::system_clock::now();
		Move move = ply.move(0,0);
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

void Game::execute(Move &move)
{
	if(!move.target || move.captures.empty())
	{
		moveless++;
		return;
	} else
		moveless = 0;

	if(move.target->type == Cell::Type::VOID)
		throw std::runtime_error("wrong target");

	if(move.override)
	{
		if(move.player.overrides == 0)
			throw std::runtime_error("no overrides left");

		move.player.overrides--;
		stats.overrides++;
	}

	Cell::Type targetCell = move.target->type; // first make a complete move...

	move.target->type = move.player.color;

	for(Cell* c: move.captures)
		c->type = move.player.color;


	switch(targetCell) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		switch(move.bonus)
		{
		case Move::BOMB:
			move.player.bombs++;
			break;
		case Move::OVERRIDE:
			move.player.overrides++;
			break;
		default:
			break;
		}
		break;
	case Cell::Type::CHOICE:
	{
		Player& swap = *move.choice;
		if(move.player.color == swap.color)
			break;

		for(Cell& c: getMap())
		{
			if(c.type == move.player.color)
				c.type = swap.color;
			else if(c.type == swap.color)
				c.type = move.player.color;
		}
	}
		break;
	case Cell::Type::INVERSION:
		for(Cell& c: getMap())
		{
			if(c.isPlayer())
				c.type = ply2type(type2ply(c.type) % players.size());
		}

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

	players.reserve(defaults.players);

	// Bombs
	auto tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("bombs delimiter not found"); // TODO: better error messages placement

	defaults.bombs = stoi(tmp[0]);
	defaults.bombsStrength = stoi(tmp[1]);

	map = new Map(Map::load(file));
}
