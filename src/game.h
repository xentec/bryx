#pragma once

#include "global.h"
#include "map.h"

struct Move
{
	Cell& from, to;
};

struct Game
{
	u32 players;
	u32 overrides;
	u32 bombs;
	u32 bombsStrength;

	Map* map;

	~Game();
	bool isValidMove(Move& move);

	static Game load(string filename);
};

