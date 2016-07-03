#pragma once

#include "global.h"

#include "cell.h"
#include "move.h"

#include <fmt/ostream.h>
#include <deque>
#include <list>


using PossibleMoves = std::deque<Move>;

struct Game;

struct Player
{
	Player(Game& game, Cell::Type color, const string& name = "");
	Player(const Player& other);
	virtual ~Player();

	Player& operator =(const Player& other);
	virtual Player* clone() const = 0;

	bool operator ==(const Player& other);
	bool operator !=(const Player& other);

	string asString() const;

	virtual Move move(PossibleMoves& moves, u32 time, u32 depth) = 0;
	virtual Move bomb(u32 time) = 0;

	std::list<Cell*> stones();
	PossibleMoves possibleMoves();

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
