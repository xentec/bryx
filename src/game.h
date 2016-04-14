#pragma once

#include "global.h"
#include "map.h"

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

	std::vector<Move> possibleMoves() const;
	std::vector<Move> possibleMovesOn(Cell& cell) const;
	Move::Error testMove(Move& move) const;

	static Game load(string filename);
private:
	Game();
};

