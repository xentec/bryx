#pragma once

#include "global.h"
#include "cell.h"

#include <vector>

struct Cell;
struct Game;
struct Move;

struct Player
{
	Player(const string& name);
	Player& operator =(const Player& other);
	virtual ~Player();

	virtual Move move() = 0;
	virtual Player& choice() = 0;
	virtual void bonus() = 0;

	std::vector<Move> possibleMoves();
	u32 score(bool inventory = false);

	u32 id;
	string name;

	u32 overrides;
	u32 bombs;

	Game* game;
};
