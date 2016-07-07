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
	aiData(other.aiData),
	map(nullptr),
	currPly(other.currPly), moveless(other.moveless)
{
	throw std::runtime_error("game copy");

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
	do {
		// "..size() + " needed or an underflow will happen
		currPly = (players.size() + currPly - 1) % players.size();
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

		print("##########");
		for(usz i = 0; i < ply.name.size(); i++)
			print("#");
		println();

		println("Stones: {}", ply.stones().size());
		println("Overrides: {}", ply.overrides);
		println("Bombs: {}", ply.bombs);


		PossibleMoves moves = ply.possibleMoves();

		if(moves.empty())
		{
			println("No moves");
			moveless++;
			nextPlayer();
			continue;
		}
		moveless = 0;

		{
			u32 ovr = 0, num = moves.size();
			for(Move& m: moves)
				if(m.override)
					ovr++;

			println("Moves: {} ({})", num-ovr, num);
			println();
		}

		start = Clock::now();
		Move move = ply.move(moves,0,2);
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

	println();
	println("BOMB phase has begun!");
	println();

	phase = Phase::BOMB;
	moveless = 0;

	do
	{
		Player& ply = currPlayer();

		println();
		println("Player {}", ply);

		print("##########");
		for(usz i = 0; i < ply.name.size(); i++)
			print("#");
		println();

		println("Bombs: {}", ply.bombs);
		println();

		if(ply.bombs == 0)
		{
			moveless++;
			nextPlayer();
			continue;
		}
		moveless = 0;

		Move move = ply.bomb(0);
		move.print();
		execute(move);
	}
	while(!hasEnded());
}

void Game::execute(Move &move)
{
#if SAFE_GUARDS
#if SAFE_GUARDS > 2
	map->check();
#endif

	if(move.getPlayer() != currPlayer())
		throw std::runtime_error("wrong player turn");

	if(!move.target)
		throw std::runtime_error("null target");
#endif

	MoveBackup backup { move, move.target->type, {} };

	if(phase == Phase::BOMB)
	{
#if SAFE_GUARDS
		if(move.target->type == Cell::Type::VOID)
			throw std::runtime_error("bombing void cell");
#endif
		for(Cell* c: move.captures)
		{
//			backup.captures.emplace_back(c->pos, c->type);
			c->clear();
		}

		move.getPlayer().bombs--;
	} else
	{

#if SAFE_GUARDS
		if(move.target->type == Cell::Type::VOID)
			throw std::runtime_error("wrong target");
#endif

		if(move.override)
		{
#if SAFE_GUARDS
			if(move.getPlayer().overrides == 0)
				throw std::runtime_error("no overrides left");
#endif
			move.getPlayer().overrides--;
			stats.overrides++;
		}


		move.target->type = move.getPlayer().color;

		for(Cell* c: move.captures)
		{
#if SAFE_GUARDS
			if(c->type == move.getPlayer().color)
				throw std::runtime_error("invalid player capture");
#endif
			backup.captures.emplace_back(c->pos, c->type);
			c->type = move.getPlayer().color;
		}

		handleSpecial(backup);

	}

#if SAFE_GUARDS > 2
	map->check();
#endif

	backup.move = move;

	moveLog.push(backup);
	stats.moves++;

	nextPlayer();
}

void Game::undo()
{
	MoveBackup& backup = moveLog.top();
	Move& move = backup.move;

	prevPlayer();

	handleSpecial(backup, true);

	move.target->type = backup.targetType;
	for(auto c : backup.captures)
		map->at(c.first).type = c.second;

	if(move.target->isCaptureable())
	{
		move.getPlayer().overrides++;
		stats.overrides--;
	}

#if SAFE_GUARDS > 2
	map->check();
#endif

	stats.moves--;
	moveLog.pop();
}

void Game::handleSpecial(MoveBackup& mb, bool undo)
{
	Move& move = mb.move;
	i32 rev = undo ? -1 : 1;

	switch(mb.targetType) // ...then look at special cases
	{
	case Cell::Type::BONUS:
		move.target->staticValue -= BONUS_VALUE*rev;
		for(Cell::Transition& tr: *move.target)
		{
			if(tr.to)
				tr.to->staticValue += BONUS_VALUE*rev;
		}
		switch(move.bonus)
		{
		case Move::BOMB:
			move.getPlayer().bombs += rev;
			break;
		case Move::OVERRIDE:
			move.getPlayer().overrides += rev;
			break;
		default:
			break;
		}
		break;
	case Cell::Type::CHOICE:
		move.target->staticValue -= CHOICE_VALUE*rev;
		for(Cell::Transition& tr: *move.target)
		{
			if(tr.to)
				tr.to->staticValue += CHOICE_VALUE*rev;
		}

		if(move.getPlayer().color == move.choice)
			break;

		for(Cell& c: getMap())
		{
			if(c.type == move.getPlayer().color)
				c.type = move.choice;
			else if(c.type == move.choice)
				c.type = move.getPlayer().color;
		}
		break;
	case Cell::Type::INVERSION:
		for(Cell& c: getMap())
		{
			if(c.isPlayer())
				// "..size() + " needed or an underflow will happen
				c.type = ply2type((defaults.players + type2ply(c.type)+rev) % defaults.players);
		}

		stats.inversions += rev;
		break;
	default:
		break;
	}
}

Move& Game::getLastMove()
{
	return moveLog.top().move;
}

void Game::printInfo() const
{

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

	aiData.amountMoves += defaults.players * defaults.overrides;

	// Evaluate cells
	//#################

	for(Cell &c: *map)
	{
		if(c.type == Cell::Type::VOID)
			continue;

		u32 totalCounter = 0, maxCounter = 0, counter = 0;
		for(usz j = 0; j < (2 * Direction::_LAST); j++)       //checks for walls twice, to get the length of the wall
		{
			Cell::Transition& tr = c.getNeighbor(Direction(j % Direction::_LAST));
			if(tr.to == nullptr)   //which is contained in maxCount
			{
				counter++;

			}else
			{
				if(counter > maxCounter)
					maxCounter = counter;

				totalCounter += counter;
				counter = 0;
			}
		}
		totalCounter /= 2;
		//maxCounter decides the maximum length of the wall
		//totalCounter decides the total amount of walls
		switch(maxCounter)
		{
			case 1:                             //at least 1 wall with no neighbour-walls
				switch(totalCounter){
					case 2:                     //-||- with a total of 2 attached walls
						c.staticValue += 3;
						break;
					case 3:                     //-||- with a total of 3 attached walls
						c.staticValue += 4;
						break;
					case 4:                     //-||- with a total of 4 attached walls
						c.staticValue += 3;
						break;
					default:                    //exact 1 wall or unexpected amount of attached walls
						c.staticValue += 2;
						break;
				}
				break;
			case 2:                             //at least 2 neighbour-walls
				switch(totalCounter){
					case 3:                     //...
						c.staticValue += 4;
						break;
					case 4:
						c.staticValue += 5;
						break;
					default:
						c.staticValue += 3;
						break;
				}
				break;
			case 3:
				switch(totalCounter){
					case 4:
						c.staticValue += 6;
						break;
					case 5:
						c.staticValue += 4;
						break;
					case 6:
						c.staticValue += 3;
						break;
					default:
						c.staticValue += 5;
						break;
				}
				break;
			case 4:
				switch(totalCounter){
					case 5:
						c.staticValue += 8;
						break;
					case 6:
						c.staticValue += 6;
						break;
					case 7:
						c.staticValue += 4;
						break;
					default:
						c.staticValue += 10;
						break;
				}
				break;
			case 5:
				switch(totalCounter){
					case 6:
						c.staticValue += 6;
						break;
					default:
						c.staticValue += 7;
						break;
				}
				break;
			case 6:
				c.staticValue += 4;
				break;
			case 7:
				c.staticValue += 2;
				break;
			default:
				c.staticValue += 1;
				break;
		}

		if(c.type == Cell::Type::BONUS)
		{
			c.staticValue += BONUS_VALUE;
			for(Cell::Transition& tr: c)
			{
				if(tr.to)
					tr.to->staticValue -= BONUS_VALUE;
			}
		}
		else if(c.type == Cell::Type::CHOICE)
		{
			c.staticValue += CHOICE_VALUE;
			for(Cell::Transition& tr: c)
			{
				if(tr.to)
					tr.to->staticValue -= CHOICE_VALUE;
			}
		}

		aiData.amountMoves++;
	}


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
