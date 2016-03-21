#pragma once

#include "global.h"

#include <vector>

struct Map
{
	Map(u32 width, u32 width);

	char at(u32 x, u32 y) const;
	char& at(u32 x, u32 y);

	void clear();
private:
	const u32 width, height;
	std::vector<std::vector<char>> data;
};
