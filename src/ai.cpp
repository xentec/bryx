#include "ai.h"

#include "game.h"
#include "map.h"

u32 AI::nameNum = 1;

AI::AI(Game &game):
	Player(game, fmt::format("bryx {}", nameNum++))
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

	return move;
}

Player& AI::choice()
{
	// TODO: better choice algo
	return **std::max_element(game.getPlayers().begin(), game.getPlayers().end(), [](Player* a, Player* b) { return a->score() < b->score(); });
}

void AI::bonus()
{
	if(/* TODO: extremly complex calculation */ 0 % 2)
		bombs++;
	else
		overrides++;
}
