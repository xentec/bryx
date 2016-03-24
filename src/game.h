#pragma once

#include "global.h"

struct Map;

struct Game
{
	u32 players;
	u32 overrides;
	u32 bombs;
	u32 bombsStrength;

	Map* map;
};
