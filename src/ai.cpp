#include "ai.h"

#include "game.h"

u32 AI::nameNum = 1;

AI::AI(): Player(fmt::format("bryx {}", nameNum++))
{}

AI::~AI()
{}

Move AI::move()
{
	std::vector<Move> moves = possibleMoves();

	u32 stoneSum = 0;
	u32 stoneMax = 0;
	std::vector<Move*> bestMoves;
	for(Move& m: moves)
	{
		if(m.override)
			continue;

		int stones = m.stones.size();
		stoneSum += stones;
		if(stones > stoneMax)
		{
			bestMoves.clear();
			stoneMax = stones;
			bestMoves.push_back(&m);
			continue;
		}
		if(stones == stoneMax)
			bestMoves.push_back(&m);
	}

	Move move  { *this, nullptr, Direction::N };

#if RANDOM
	if(bestMoves.size())
	{
		move = moves[moves.size() > 1 ? rand() % (moves.size()-1) : 0];
	}
#else
	if(bestMoves.size())
	{
		move = *bestMoves[0];
//		move = *bestMoves[bestMoves.size() > 1 ? rand() % (bestMoves.size()-1) : 0];
	}
#endif

	return move;
}

Player& AI::choice()
{
	// TODO: better choice algo
	return *game->players[rand() % game->players.size()];
}

void AI::bonus()
{
	if(/* TODO: extremly complex calculation */ rand() % 2)
		bombs++;
	else
		overrides++;
}
