#pragma once

#include "global.h"
#include "vector.h"

#include <vector>

enum class Cell : u8
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

enum class Direction : u8
{
	N, NE, E, SE, S, SW, W, NW
};

struct Location
{
	vec2 pos;
	Direction dir;
};

struct Transistion
{
	Transistion(const Location& from, const Location& to);

	const Location from, to;
};

struct Map
{
	Map(u32 width, u32 height);

	void clear(Cell new_cell = Cell::EMPTY);
	void add(const Transistion& trn);

	Cell at(u32 x, u32 y) const;
	Cell& at(u32 x, u32 y);

	struct Row
	{
		Cell& operator[](usz index);
	private:
		Row(Map& map, usz offset);
		const usz offset;
		Map& map;

		friend class Map;
	};
	Row operator[](usz index);

	string asString();


	const u32 width, height;
private:
	std::vector<Cell> data;
	std::vector<Transistion> trans;
};
