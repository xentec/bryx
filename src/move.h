#pragma once

#include "global.h"

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
	};

	enum Bonus
	{
		BOMB = 20, OVERRIDE = 21
	};

	Move(Player& player, Cell* target);
	Move& operator =(const Move& other);

	string asString() const;
	void print() const;

	Player& player;

	Cell *target;
	bool override;
	Bonus bonus;
	Player* choice;

	Error err;
	std::vector<std::list<Cell*> > captures;


	static string err2str(Move::Error err);
};
