#pragma once

#include "global.h"
#include "console.h"
#include "vector.h"

#include <array>

enum Direction : u8
{
	N, NE, E, SE, S, SW, W, NW, _LAST
};
vec dir2vec(Direction dir);
string dir2str(Direction dir);
inline string dir2str(u32 i) { return dir2str((Direction) i); }
inline Direction dir180(Direction in) { return (Direction) ((in+Direction::_LAST/2) % (Direction::_LAST)); }

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
		Direction entry;
	};


	Cell(Map &map, vec pos, Type type);
	Cell(Cell&& other) = default;

	bool operator ==(const Cell& other) const;
	bool operator !=(const Cell& other) const;

	string asString() const;

	bool isFree() const;
	bool isPlayer() const;
	bool isCaptureable() const;
	bool isSpecial() const;

	Cell* getDirectNeighbor(Direction dir) const;
	Cell* getNeighbor(Direction& dir) const;
	void addTransistion(Direction exit, Direction entry, Cell* target);

	console::Format getFormat() const;

	static bool isValid(char ch);


	const vec pos;
	Cell::Type type;
	std::array<Transition, 8> transitions;

	Map& map;
};

inline u8 type2ply(Cell::Type type) { return type - Cell::Type::P1; }
inline Cell::Type ply2type(u8 ply_id) { return static_cast<Cell::Type>(ply_id + Cell::Type::P1); }
