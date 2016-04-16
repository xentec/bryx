#pragma once

#include "global.h"
#include "map.h"

#include <vector>


struct Game;
struct Move;

struct Player
{
	Player(Game& game, Cell::Type type);
	Player& operator =(const Player& other);

	std::vector<Move> possibleMoves();
	std::vector<Move> possibleMovesOn(Cell& cell);

	Player& choice() const;
	void bonus();

	Cell::Type color;
	u32 bombs;
	u32 overrides;

	Game& game;
};
