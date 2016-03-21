#pragma once

#include "global.h"

#include <vector>

enum class Cell : char
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

struct Map
{
	Map(u32 width, u32 height);

	Cell at(u32 x, u32 y) const;
	Cell& at(u32 x, u32 y);

	void clear(Cell cell = Cell::EMPTY);
private:
	const u32 width, height;
	std::vector<Cell> data;
};
