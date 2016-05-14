#pragma once

#include "global.h"
#include "cell.h"

#include <vector>

struct Game;
struct Move;

struct Player
{
	Player(Game& game, Cell::Type color, const string& name = "");
	Player& operator =(const Player& other);
	virtual ~Player();

	virtual Move move() = 0;

	std::vector<Cell*> stones();
	std::vector<Move> possibleMoves();

	void evaluate(Move& move) const;

	string name;

	Cell::Type color;
	u32 overrides;
	u32 bombs;

	Game& game;
};
