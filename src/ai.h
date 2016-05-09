#pragma once

#include "vector.h"
#include "player.h"

struct AI : Player
{
	AI(Game& game, u32 id); // TODO: maybe difficulty setting
	virtual ~AI();

	virtual Move move();
	virtual void bonus();
	virtual Player& choice();
};
