#pragma once

#include "player.h"


struct Human : Player
{
	Human(Game& game, Cell::Type color);
	virtual ~Human();

	virtual Move move(u32, u32);
};
