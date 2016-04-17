#pragma once

#include "global.h"
#include "map.h"

#include <vector>


struct Game;
struct Move;

struct Player
{
	Player();
	Player& operator =(const Player& other);
	virtual ~Player();

	virtual Move move() = 0;
	virtual Player& choice() = 0;
	virtual void bonus() = 0;

	std::vector<Move> possibleMoves();
	std::vector<Move> possibleMovesOn(Cell& cell);

	u32 id;

	u32 overrides;
	u32 bombs;

	Game* game;
};
