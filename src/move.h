#pragma once

#include "global.h"
#include "map.h"

#include <list>
#include <vector>

struct Game;
struct Player;

struct Move
{
	enum class Error
	{
		NONE,
		WRONG_START,
		NO_CONNECTIONS,
		NO_STONES_CAPTURED,
	};

	Move(Player& player, Cell* target, bool override);
	Move(const Move& other);
	Move& operator =(const Move& other);

	u32 score() const;

	string asString() const;
	void print() const;

	Player& player;

	Cell *target;
	Error err;

	std::vector<std::list<Cell*> > captures;
	bool override;

	static string err2str(Move::Error err);
};
