#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>
#include <numeric>
#include <map>

AI::AI(Game &game, Cell::Type color):
	Player(game, color, "bryx")
{}

AI::AI(const AI &other):
	Player(other)
{}

AI::~AI()
{}

Player *AI::clone() const
{
	return new AI(*this);
}

static Heuristic
	infMin = std::numeric_limits<Heuristic>::min(),
	infMax = std::numeric_limits<Heuristic>::max();

inline bool minPrune (Heuristic h, Heuristic& a, Heuristic& v, Heuristic& b){ if(h < v) { v = h; if(v < b) b = v; } return v <= a; }; // min
inline bool maxPrune (Heuristic h, Heuristic& a, Heuristic& v, Heuristic& b){ if(h > v) { v = h; if(v > a) a = v; } return v >= b; }; // max

Move AI::move(std::list<Move>& posMoves, u32 time, u32 depth)
{
	// sorting
	std::map<Heuristic, Move> moves;
	for(Move& m: posMoves)
	{
		if(m.override && overrides == 0) // TODO: Remove condition
			continue;

		game.execute(m, true);
		moves.emplace(evalState(game), m);
		game.undo(m);
	}

	std::pair<Heuristic, Move> bestMove = { infMin, moves.rbegin()->second };
	Heuristic a = infMin, v = infMin, b = infMax;

	bool run = true;
	for(auto iter = moves.rbegin(); iter != moves.rend(); iter++)
	{
		Move& m = iter->second;

		game.execute(m, true);
		game.nextPlayer();

		Heuristic h = bestState(game, depth-1, a, b);

		game.prevPlayer();
		game.undo(m);

		run = !maxPrune(h, a, v, b);

		if(v > bestMove.first)
			bestMove = {v, m};

		if(!run) break;
	}

	if(bestMove.second.target == nullptr)
		throw std::runtime_error("no move");

	Move& move = bestMove.second;

	if(move.target)
	{
		switch(move.target->type)
		{
		case Cell::Type::BONUS:
			// TODO: better choice algo
			move.bonus = Move::Bonus::OVERRIDE;
			break;
		case Cell::Type::CHOICE:
			// TODO: better choice algo
			{
			  Player* c =
				*std::max_element(game.getPlayers().begin(), game.getPlayers().end(),
					[](Player* a, Player* b) { return a->stones().size() < b->stones().size();
				});
			  move.choice = c->color;
			}
			break;
		default:
			break;
		}
	}

	return move;
}

#include "clock.h"

Heuristic AI::bestState(Game& state, u32 depth, Heuristic& a, Heuristic& b)
{
	Player& ply = state.currPlayer();

	// sorting
	std::map<Heuristic, Move> moves;
	for(Move& m: ply.possibleMoves())
	{
		if(m.override && ply.overrides == 0) // TODO: Remove condition
			continue;

		state.execute(m, true);
		moves.emplace(evalState(state), m);
		state.undo(m);
	}

	u32 d = depth - 1;
	Heuristic v;

	if(ply.color != color) // min
	{
		v = infMax;
		if(d == 0)
			// reuse evalution in map key
			for(auto& pair: moves)
			{
				if(minPrune(pair.first, a, v, b)) break;
			}
		else
			for(auto& pair: moves)
			{
				Move& m = pair.second;

				state.execute(m, true);
				state.nextPlayer();

				Heuristic h = bestState(state, d, a, b);

				state.prevPlayer();
				state.undo(m);

				if(minPrune(h, a, v, b)) break;
			}
	} else
	{
		v = infMin;
		if(d == 0)
			// reuse evalution in map key
			for(auto iter = moves.rbegin(); iter != moves.rend(); iter++)
			{
				if(minPrune(iter->first, a, v, b)) break;
			}
		else
			for(auto iter = moves.rbegin(); iter != moves.rend(); iter++)
			{
				Move& m = iter->second;

				state.execute(m, true);
				state.nextPlayer();

				Heuristic h = bestState(state, d, a, b);

				state.prevPlayer();
				state.undo(m);

				if(minPrune(h, a, v, b)) break;
			}
	}

	return v;
}

Heuristic AI::evalState(const Game& state) const
{
	Heuristic h = 0;

	Player& futureMe = *state.getPlayers()[type2ply(color)];

	h += 20 * futureMe.overrides;
	h += futureMe.bombs * state.defaults.bombsStrength;

	for(Cell& c: state.getMap())
	{
		if(c.type == color)
		{
            h = c.staticValue + 5;

            for(Cell::Transition nc : c)
			{
				if(nc.to)
				{
					if(nc.to->isSpecial())
						h -= 50;
                }
			}
			// #################

		} else if(c.isPlayer())
		{
            h = -c.staticValue - 5;
		}
	}

	return h;
}
