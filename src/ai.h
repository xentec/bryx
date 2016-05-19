#pragma once

#include "vector.h"
#include "player.h"

struct AI : Player
{
	AI(Game& game, Cell::Type color); // TODO: maybe difficulty setting
	AI(const AI& other);
	virtual ~AI();

	virtual Player* clone() const;
	virtual Move move(u32 time, u32 depth);
};
