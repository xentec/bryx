#pragma once

#include "game.h"

struct Game;
struct Player
{
	Player(Game& game, Cell::Type type);
	Player& operator =(const Player& other);

	Cell::Type me;
	Game& game;
};
