#pragma once

#include "player.h"


struct Human : Player
{
	Human(const string& name);
	virtual ~Human();

	virtual Move move();
	virtual void bonus();
	virtual Player& choice();
};
