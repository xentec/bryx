#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>
#include <numeric>


static i32 scoreOf(const Move& move)
{
	return std::accumulate(move.captures.begin(), move.captures.end(),
							0, [](i32 sum, std::list<Cell*> line)
	{
		return sum + line.size();
	});
}


AI::AI(Game &game, Cell::Type color):
	Player(game, color, fmt::format("bryx {}", color))
{}

AI::~AI()
{}

Move AI::move()
{
	std::vector<Move> moves = possibleMoves();

	i32 scoreSum = 0;
	i32 scoreMax = 0;
	std::vector<Move*> bestMoves;
	for(Move& m: moves)
	{
		i32 score = scoreOf(m);

		scoreSum += score;
		if(score > scoreMax)
		{
			if(!m.override)
				bestMoves.clear();
			scoreMax = score;
			bestMoves.push_back(&m);
			continue;
		}
		if(score == scoreMax && m.override == bestMoves.front()->override)
			bestMoves.push_back(&m);
	}

	Move move  { *this, nullptr };

	if(bestMoves.size())
	{
		std::sort(bestMoves.begin(), bestMoves.end(),
			[=](Move* a, Move* b)
			{
				return scoreOf(*a) > scoreOf(*b);
			});

		move = *bestMoves.front();
	}

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
			 move.choice =
				*std::max_element(game.getPlayers().begin(), game.getPlayers().end(),
					[](Player* a, Player* b) { return a->stones().size() < b->stones().size();
				});
			break;
		default:
			break;
		}
	}

	return move;
}
