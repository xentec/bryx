#pragma once

#include "vector.h"
#include "player.h"

using Heuristic = i32;

struct AI : Player
{
	AI(Game& game, Cell::Type color); // TODO: maybe difficulty setting
	AI(const AI& other);
	virtual ~AI();

	virtual Player* clone() const;
	virtual Move move(std::list<Move>& moves, u32 time, u32 depth);

	Heuristic evalState(const Game &state) const;
	Heuristic bestState(Game& state, u32 depth, Heuristic &a, Heuristic &b);
};
