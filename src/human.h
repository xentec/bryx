#pragma once

#include "player.h"


struct Human : Player
{
	Human(Game& game, Cell::Type color);
	Human(const Human& other);
	virtual ~Human();

	virtual Player* clone() const;
	virtual Move move(PossibleMoves&, u32, u32);
	virtual Move bomb(u32 time);
};
