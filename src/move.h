#pragma once

#include "global.h"

#include "cell.h"

#include <list>
#include <vector>

struct Cell;
struct Game;
struct Player;

struct Move
{
	enum class Error
	{
		NONE = 0,
		WRONG_START,
		NO_CONNECTIONS,
		NO_OVERRIDES,
	};

	enum Bonus
	{
		NONE = 0, BOMB = 20, OVERRIDE = 21
	};

	Move(Player& player, Cell* target);
	Move& operator =(const Move& other);

	string asString() const;
	void print() const;

	Player& player;

	Cell *target;
	bool override;
	Bonus bonus;
	Cell::Type choice;

	Error err;
	std::list<Cell*> captures;

	struct Backup
	{
		std::list<std::pair<vec, Cell::Type> > captures;
		Cell::Type target;
	} backup;

	static string err2str(Move::Error err);
};
