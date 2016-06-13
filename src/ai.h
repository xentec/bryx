#pragma once

#include "vector.h"
#include "player.h"
#include "util/clock.h"

#include <deque>

using Quality = isz;

struct AIMove
{
	Quality score;
	Move move;
};

struct AI : Player
{
	AI(Game& game, Cell::Type color); // TODO: maybe difficulty setting
	AI(const AI& other);
	virtual ~AI();

	virtual Player* clone() const;
	virtual Move move(PossibleMoves &moves, u32 time, u32 depth);

	Quality evalState(Game &state, Move& m) const;
	AIMove bestState(Game& state, PossibleMoves &posMoves, u32 depth, Quality& a, Quality& b);
	Move bestState2(Game &game);
private:
	u32 maxDepth;
	TimePoint endTime;

	std::deque<Move> moveChain;

	Duration evaltime;
};
