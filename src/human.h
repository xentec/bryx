#pragma once

#include "player.h"


struct Human : Player
{
	Human(Game& game, u32 id);
	virtual ~Human();

	virtual Move move();
};
