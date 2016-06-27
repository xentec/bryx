#include "game.h"

#include "map.h"
#include "ai.h"
#include "util/clock.h"

#include <fmt/format.h>
#include <list>

#define VERBOSE 1

Game::Game():
	defaults{ 0, 0, 0, 0 }, stats { 0, 0, 0, {} },
    aiData{ 0, 15, {}, false, 0 },
	phase(Phase::REVERSI),
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
	do{
		currPly = (currPly+1) % players.size();
	} while(players[currPly] == nullptr);

	return currPlayer();
}

Player& Game::prevPlayer()
{
	do{
		currPly = (currPly-1) % players.size();
	} while(players[currPly] == nullptr);

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

usz Game::getMoveNum() const
{
    return moveLog.size();
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
		Move move = ply.move(moves,0,5);
		end = Clock::now();

		elapsed = end-start;
		if(elapsed > stats.time.moveMax)
			stats.time.moveMax = elapsed;

		stats.time.moveAvg += elapsed/(num*(num+1));
		num++;

		move.print();
		execute(move);
	}
	while(!hasEnded());
}

void Game::execute(Move &move)
{
#if SAFE_GUARDS
	map->check();

	if(move.player != currPlayer())
		throw std::runtime_error("wrong player turn");

	if(!move.target)
		throw std::runtime_error("null target");
#endif

	if(phase == Phase::BOMB)
	{
#if SAFE_GUARDS
		if(move.target->type == Cell::Type::VOID)
			throw std::runtime_error("bombing void cell");
#endif
		std::vector<Cell*> field = map->getQuad(move.target->pos, defaults.bombsStrength);
		for(Cell* c: field)
			c->clear();

		return;
	}

#if SAFE_GUARDS
	if(move.target->type == Cell::Type::VOID)
		throw std::runtime_error("wrong target");
#endif

	if(move.override)
	{
#if SAFE_GUARDS
		if(move.player.overrides == 0)
			throw std::runtime_error("no overrides left");
#endif
		move.player.overrides--;
		stats.overrides++;
	}

	MoveBackup backup { move, move.target->type, {} };

	Cell::Type targetCell = move.target->type; // first make a complete move...
	move.target->type = move.player.color;

	for(Cell* c: move.captures)
	{
#if SAFE_GUARDS
		if(c->type == move.player.color)
			throw std::runtime_error("invalid player capture");
#endif
		backup.captures.emplace_back(c->pos, c->type);
		c->type = move.player.color;
	}

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
#if SAFE_GUARDS
	map->check();
#endif

	moveLog.push(backup);
	stats.moves++;

	nextPlayer();
}

void Game::undo()
{
	MoveBackup& backup = moveLog.top();
	Move& move = backup.move;

	prevPlayer();

	switch(backup.targetType) // ...then look at special cases
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

	move.target->type = backup.targetType;
	for(auto c : backup.captures)
		map->at(c.first).type = c.second;

	if(move.target->isCaptureable())
	{
		move.player.overrides++;
		stats.overrides--;
	}

#if SAFE_GUARDS
	map->check();
#endif

	stats.moves--;
	moveLog.pop();
}

Move& Game::getLastMove()
{
	return phase == Phase::REVERSI ? moveLog.top().move : moveLog.top().move;
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

	// AI static values

	aiData.bombValue = (((defaults.bombsStrength * 2 + 1) * (defaults.bombsStrength * 2 + 1))
						/ (map->width * map->height)) * 100;
    for(Cell &c: *map){
        if(c.type == Cell::Type::VOID)
            continue;

        aiData.amountMoves++;
    }
    aiData.amountMoves += defaults.players * defaults.overrides;

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
