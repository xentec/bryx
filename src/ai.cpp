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

static auto	minPrune = [](Heuristic& h, Heuristic& a, Heuristic& v, Heuristic& b){ if(h < v) { v = h; if(v < b) b = v; } return v <= a; }; // min
static auto	maxPrune = [](Heuristic& h, Heuristic& a, Heuristic& v, Heuristic& b){ if(h > v) { v = h; if(v > a) a = v; } return v >= b; }; // max

Move AI::move(const std::list<Move>& moves, u32 time, u32 depth)
{
	std::pair<Heuristic, Move> bestMove = { infMin, moves.front() };

	Heuristic a = infMin, v = infMin, b = infMax;

	for(Move m: moves)
	{
		if(m.override && overrides == 0)
			continue;

		game.execute(m, true);
		game.nextPlayer();
		Heuristic h = bestState(game, depth-1, a, b);
		game.prevPlayer();
		game.undo(m);

		if(maxPrune(h, a, v, b))
			break;

		if(h > bestMove.first)
			bestMove = {h, m};
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

Heuristic AI::bestState(Game& state, u32 depth, Heuristic& a, Heuristic& b)
{
	if(depth == 0)
		return evalState(state);

	Player& ply = state.currPlayer();
	bool min = ply.color != color;

	Heuristic v = min ? infMax : infMin;

	auto prune = min ? minPrune : maxPrune;

	for(Move& m: ply.possibleMoves())
	{
		if(m.override && ply.overrides == 0)
			continue;

		state.execute(m, true);
		state.nextPlayer();
		Heuristic h = bestState(state, depth-1, a, b);
		state.prevPlayer();
		state.undo(m);

		if(prune(h, a, v, b))
			break;
	}

	return v;
}

Heuristic AI::evalState(const Game& state) const
{
	Heuristic h = 0;

	for(Cell& c: state.getMap())
	{
		if(c.type == color)
		{
			h += 5;

			// Corners
			u32 corner = 0;
			for(u32 dir = Direction::N; dir < Direction::_LAST; dir++)
			{
				Direction d = (Direction) dir;
				Cell* nc = c.getNeighbor(d);
				if(nc)
				{
					if(nc->isSpecial())
						h -= 50;
				} else
					corner++;
			}
			if(corner > 4 && corner < 7)
				h *= corner;
			// #################

		} else if(c.isPlayer())
		{
			h -= 5;
		}
	}

	return h;
}
