#include "game.h"

#include "clock.h"
#include "map.h"
#include "util.h"
#include "ai.h"

#include <fmt/format.h>

#include <list>

#define VERBOSE 1

Game::Game():
	defaults{ 0, 0, 0, 0 }, stats { 0, 0, 0, {} },
	map(nullptr),
	currPly(0), moveless(0)
{}

Game::Game(const Game &other):
	defaults(other.defaults), stats(other.stats),
	map(nullptr),
	currPly(other.currPly), moveless(other.moveless)
{
	if(other.map)
		map = new Map(*other.map);

	for(Player* p: other.players)
	{
		Player* pc = p->clone();
		pc->game = *this;
		players.push_back(pc);
	}
}

Game::~Game()
{
	for(Player* ply: players)
		delete ply;

	delete map;
}

Player& Game::currPlayer() const
{
	return *players[currPly];
}

Player& Game::nextPlayer()
{
	currPly = (currPly+1) % players.size();
	return currPlayer();
}

Player& Game::prevPlayer()
{
	currPly = (currPly-1) % players.size();
	return currPlayer();
}

Map& Game::getMap() const
{
	return *map;
}

std::vector<Player*>& Game::getPlayers()
{
	return players;
}

std::vector<Player*> Game::getPlayers() const
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

	TimePoint start, end;
	Duration elapsed;
	u32 num = 1;

	do
	{
		Player& ply = currPlayer();

		println();
		println("Player {}", ply);

		println("##########");
		for(usz i = 0; i < ply.name.size(); i++)
			print("#");
		println();

		println("Overrides: {}", ply.overrides);
		println("Bombs: {}", ply.bombs);
		println();

#if MOVES_ITERATOR
		std::deque<Move> moves = ply.possibleMoves().all();
#else
		std::deque<Move> moves = ply.possibleMoves();
#endif
		if(moves.empty())
		{
			println("No moves");
			moveless++;
			continue;
		}

		{
			u32 ovr = 0, num = moves.size();
			for(Move& m: moves)
				if(m.override)
					ovr++;

			println("Moves: {} ({})", num-ovr, num);
			println();

#if 0
			for(Move m: moves)
			{
				m.print();
				println();
			}
#endif
		}

		start = Clock::now();
		Move move = ply.move(moves,0,3);
		end = Clock::now();

		elapsed = end-start;
		if(elapsed > stats.time.moveMax)
			stats.time.moveMax = elapsed;

		stats.time.moveAvg += elapsed/(num*(num+1));
		num++;

		move.print();
		execute(move);

		nextPlayer();

	}
	while(!hasEnded());
}

void Game::execute(Move &move, bool backup)
{
	if(!move.target)
		throw std::runtime_error("empty target");

	if(move.target->type == Cell::Type::VOID)
		throw std::runtime_error("wrong target");

	if(move.override)
	{
		if(move.player.overrides == 0)
			throw std::runtime_error("no overrides left");

		move.player.overrides--;
		stats.overrides++;
	}

	if(backup)
	{
		move.backup.target = move.target->type;
		for(Cell* c: move.captures)
			move.backup.captures.emplace_back(c->pos, c->type);
	}

	Cell::Type targetCell = move.target->type; // first make a complete move...

	move.target->type = move.player.color;

	for(Cell* c: move.captures)
		c->type = move.player.color;

	switch(targetCell) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		move.target->staticValue -= BONUS_VALUE;
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
		move.target->staticValue -= CHOICE_VALUE;
		if(move.player.color == move.choice)
			break;

		for(Cell& c: getMap())
		{
			if(c.type == move.player.color)
				c.type = move.choice;
			else if(c.type == move.choice)
				c.type = move.player.color;
		}
	}
		break;
	case Cell::Type::INVERSION:
		for(Cell& c: getMap())
		{
			if(c.isPlayer())
				c.type = ply2type((type2ply(c.type)+1) % players.size());
		}

		stats.inversions++;
		break;
	default:
		break;
	}
	stats.moves++;
}

void Game::undo(Move &move)
{
	switch(move.backup.target) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		move.target->staticValue += BONUS_VALUE;
		switch(move.bonus)
		{
		case Move::BOMB:
			move.player.bombs--;
			break;
		case Move::OVERRIDE:
			move.player.overrides--;
			break;
		default:
			break;
		}
		break;
	case Cell::Type::CHOICE:
	{
		move.target->staticValue += CHOICE_VALUE;
		if(move.player.color == move.choice)
			break;

		for(Cell& c: getMap())
		{
			if(c.type == move.player.color)
				c.type = move.choice;
			else if(c.type == move.choice)
				c.type = move.player.color;
		}
	}
		break;
	case Cell::Type::INVERSION:
		for(Cell& c: getMap())
		{
			if(c.isPlayer())
				c.type = ply2type((type2ply(c.type)-1) % players.size());
		}

		stats.inversions--;
		break;
	default:
		break;
	}
	stats.moves--;

	move.target->type = move.backup.target;
	for(auto c : move.backup.captures)
		map->at(c.first).type = c.second;

	if(move.target->isCaptureable())
		move.player.overrides++;
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

	// Make some meassurements
	AI tester(*this, Cell::Type::P1);
	players.push_back(&tester);

	// calc average eval time
	TimePoint start, end;
	u32 num = 1;

	for(usz i = 0; i < 10; ++i)
	{
		start = Clock::now();
		tester.evalState(*this);
		end = Clock::now();

		aiData.evalTime += (end-start)/(num*(num+1));
		num++;
	}

	players.clear();

	println("Map: {}x{}", getMap().width, getMap().height);
	getMap().print();
	println();
	println("Players: {}", defaults.players);
	println("Overrides: {}", defaults.overrides);
	println("Bombs: {} ({})", defaults.bombs, defaults.bombsStrength);
	println();
	println("Avg Eval time: {} ms", aiData.evalTime.count());

}
