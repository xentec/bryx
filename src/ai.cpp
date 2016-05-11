#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>

AI::AI(Game &game, u32 id):
	Player(game, id, fmt::format("bryx {}", id))
{}

AI::~AI()
{}

Move AI::move()
{
	std::vector<Move> moves = possibleMoves();

	u32 scoreSum = 0;
	u32 scoreMax = 0;
	std::vector<Move*> bestMoves;
	for(Move& m: moves)
	{
		if(m.override)
			continue;

		int score = 0; // m.score();
		scoreSum += score;
		if(score > scoreMax)
		{
			if(!m.override)
				bestMoves.clear();
			scoreMax = score;
			bestMoves.push_back(&m);
			continue;
		}
		if(score == scoreMax)
			bestMoves.push_back(&m);
	}

	Move move  { *this, nullptr };

	if(bestMoves.size())
	{
		std::sort(bestMoves.begin(), bestMoves.end(),
			[=](Move* a, Move* b)
			{
				return false; // a->score() > b->score();
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
