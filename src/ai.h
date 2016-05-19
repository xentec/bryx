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
	virtual Move move(u32 time, u32 depth);

	std::pair<Heuristic, Move> bestState(Game &state, u32 depth, Heuristic a, Heuristic b);

	Heuristic evalState(const Game &state) const;
};
