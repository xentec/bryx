#pragma once

#include "player.h"


struct Human : Player
{
	Human(Game& game, Cell::Type color);
	Human(const Human& other);
	virtual ~Human();

	virtual Player* clone() const;
	virtual Move move(const std::list<Move>&, u32, u32);
};
