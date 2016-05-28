#pragma once

#include "global.h"
#include "cell.h"

#include <fmt/ostream.h>
#include <list>

struct Game;
struct Move;

struct Player
{
	Player(Game& game, Cell::Type color, const string& name = "");
	Player(const Player& other);
	virtual ~Player();

	Player& operator =(const Player& other);
	virtual Player* clone() const = 0;

	string asString() const;

	virtual Move move(std::list<Move>& moves, u32 time, u32 depth) = 0;

	std::list<Cell*> stones();
	std::list<Move> possibleMoves();

	void evaluate(Move& move) const;

	string name;

	Cell::Type color;
	u32 overrides;
	u32 bombs;

	Game& game;

	friend std::ostream& operator<<(std::ostream &os, const Player &object)
	{
		return os << object.asString();
	}
};

