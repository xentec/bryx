#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>
#include <numeric>
#include <map>


std::pair<Heuristic, Move> AI::bestState(Game& state, u32 depth, Heuristic a, Heuristic b)
{
	if(depth == 0)
		return { evalState(state), { state.currPlayer(), nullptr }};

	Player& ply = state.currPlayer();
	
	std::list<Move> moves = ply.possibleMoves();
	std::map<Heuristic, Move&> nodes;

	for(Move& m: moves)
	{
		if(m.override && ply.overrides == 0)
			continue;

		state.execute(m, true);
		state.nextPlayer();
		nodes.emplace(bestState(state, depth-1, a, b).first, m);
		state.prevPlayer();
		state.undo(m);
	}

	if(nodes.empty())
		throw std::runtime_error("no moves");

	// if player doesn't have my color, take lowest move, else hightest (std::map is a red black tree)
	return *(state.currPlayer().color != color ? nodes.begin() : --nodes.end());
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


AI::AI(Game &game, Cell::Type color):
	Player(game, color, fmt::format("bryx {}", color))
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

Move AI::move(const std::list<Move>& moves, u32 time, u32 depth)
{
	Move move = bestState(game, depth, 0, 0).second;

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
