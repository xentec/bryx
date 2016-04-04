#pragma once

#include "global.h"
#include "location.h"
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

struct Transistion
{
	Transistion(const Location& from, const Location& to);

	const Location from, to;

	string asString() const;
};

struct Map
{
	Map(u32 width, u32 height);

	void clear(Cell new_cell = Cell::EMPTY);

	Cell at(u32 x, u32 y) const;
	Cell& at(u32 x, u32 y);

	inline Cell at(Vec2 vec) const { return at(vec[0], vec[1]); }
	inline Cell& at(Vec2 vec)      { return at(vec[0], vec[1]); }

	void add(const Transistion& trn);
	std::vector<Transistion>& getTransitstions();

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

	string asString(bool color = false);

	static bool isCell(u8 c);

	const u32 width, height;
private:
	bool isValid(const Location& loc) const;
	bool checkXY(u32 x, u32 y) const;

	std::vector<Cell> data;
	std::vector<Transistion> trans;
};
