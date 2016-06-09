#pragma once

#include "vector.h"
#include "player.h"
#include "util/clock.h"

#include <deque>

using Heuristic = i32;

struct AI : Player
{
	AI(Game& game, Cell::Type color); // TODO: maybe difficulty setting
	AI(const AI& other);
	virtual ~AI();

	virtual Player* clone() const;
	virtual Move move(std::deque<Move>& moves, u32 time, u32 depth);

	Heuristic evalState(Game &state, Move& m) const;
	Heuristic bestState(Game& state, u32 depth, Heuristic &a, Heuristic &b);
	Heuristic bestState2(Game &game, u32 depth, Heuristic &a, Heuristic &b);
private:
	u32 maxDepth;
	TimePoint endTime;

	std::deque<Move> stateTree;

	Duration evaltime;
};
