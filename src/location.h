#pragma once

#include "global.h"
#include "vector.h"

enum class Direction : u8
{
	N, NE, E, SE, S, SW, W, NW
};

struct Location
{
	Location();
	Location(Vec2 pos, Direction dir);
	Location(Vec2 pos, Vec2 dir);

	Vec2 pos;
	Vec2 dir;

	string asString() const;
};
