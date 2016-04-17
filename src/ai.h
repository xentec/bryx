#pragma once

#include "player.h"

struct AI : Player
{
	AI(); // TODO: maybe difficulty setting
	virtual ~AI();

	virtual Move move();
	virtual void bonus();
	virtual Player& choice();
};
