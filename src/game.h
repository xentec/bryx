#pragma once

#include "global.h"
#include "map.h"

struct Move
{
	enum class Error
	{
		NONE,
		WRONG_START,
		TARGET_OCCUPIED,
		WRONG_PATH,
		NO_STONES_CAPTURED,
	};

	Cell &from;
	Direction dir;

	std::vector<Cell*> stones;
	bool override;

	static string err2str(Move::Error err);
};

///TODO: State analysis: check for moves and possible map expansion

struct Game
{
	Cell::Type me = Cell::Type::P1;
	u32 players;
	u32 overrides;
	u32 bombs;
	u32 bombsStrength;

	Map* map;

	~Game();
	Move::Error testMove(Move& move);

	static Game load(string filename);
	static Cell* test;

private:
	Game();
};

