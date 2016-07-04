#pragma once

#include "global.h"

#include "cell.h"

#include <deque>
#include <vector>
#include <forward_list>


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
	Move(const Move& other);
	Move& operator =(const Move& other);

	bool operator ==(const Move& other);
	bool operator !=(const Move& other);

	Player& getPlayer();

	string asString() const;
	void print() const;

	void clear();

	Player* player;

	Cell *target;
	bool override;
	Bonus bonus;
	Cell::Type choice;

	Error err;
	std::forward_list<Cell*> captures;

	friend std::ostream& operator<<(std::ostream &os, const Move &object)
	{
		return os << object.asString();
	}

	static string err2str(Move::Error err);
};
