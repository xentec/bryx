#pragma once

#include "global.h"
#include "vector.h"

#include <array>

enum Direction : u8
{
	N, NE, E, SE, S, SW, W, NW, _LAST
};
Vec2 dir2vec(Direction dir);
string dir2str(Direction dir);
inline string dir2str(u32 i) { return dir2str((Direction) i); }
Direction str2dir(string input);
inline Direction dir180(Direction in) { return (Direction) ((Direction::_LAST/2+4) % (Direction::_LAST-1)); }

struct Map;

struct Cell
{
	enum Type : u8
	{
		// standard fields
		VOID = '-',
		EMPTY = '0',

		// special
		BONUS = 'b',
		CHOICE = 'c',
		INVERSION = 'i',
		EXPANSION = 'x',

		// players
		P1 = '1', P2, P3, P4, P5, P6, P7, P8,
	};
	struct Transition
	{
		Cell* target;
		Direction out;
	};


	Cell(Map &map, Vec2 pos, Type type);

	bool operator ==(const Cell& other) const;
	bool operator !=(const Cell& other) const;

	string asString() const;

	bool isFree() const;
	bool isPlayer() const;
	bool isPlayer(i32 player) const;
	void setPlayer(i32 player);
	bool isCaptureable() const;
	bool isSpecial() const;

	Cell* getDirectNeighbor(Direction dir) const;
	Cell* getNeighbor(Direction& dir) const;
	void addTransistion(Direction in, Direction out, Cell* target);

	static bool isValid(char ch);

	const Vec2 pos;
	Cell::Type type;
	std::array<Transition, 8> transitions;

	Map& map;
};
