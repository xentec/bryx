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
	virtual Move bomb(u32 time);

	Quality evalState(Game &state) const;
	Quality evalMove(Game &state, Move& move) const;
	Quality bestState(Game& state, PossibleMoves &posMoves, u32 depth, Quality a, Quality b);
	Move bestState2(Game &game);

	void handleSpecials(Move& move) const;

	bool playerMoved(Move& move);
private:
	u32 maxDepth;
	TimePoint endTime;

	u32 states;
	u32 deepest;

	std::deque<AIMove> moveChain;
	std::deque<AIMove> movePlan;

	Duration evaltime;
};
