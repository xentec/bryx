#pragma once

#include "global.h"
#include "map.h"

#include <vector>

struct Move
{
	enum class Error
	{
		NONE,
		WRONG_START,
		LINE_FULL,
		PATH_BLOCKED,
		NO_STONES_CAPTURED,
	};

	Move(Cell& start, Direction dir);
	Move(const Move& other);
	Move& operator =(const Move& other);

	Cell &start, *end;
	Direction dir;
	Error err;

	std::vector<Cell*> stones;
	bool override;

	static string err2str(Move::Error err);
};
