#pragma once

#include "vector.h"
#include "player.h"

struct AI : Player
{
	AI(Game& game, Cell::Type color); // TODO: maybe difficulty setting
	virtual ~AI();

	virtual Move move();
};
