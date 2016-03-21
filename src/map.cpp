#include "map.h"

Map::Map(u32 width, u32 height):
	width(width), height(height), data()
{
	data.reserve(width*height);
}

Cell Map::at(u32 x, u32 y) const
{
	return data[y*height+x];
}

Cell& Map::at(u32 x, u32 y)
{
	return data[y*height+x];
}

void Map::clear(Cell new_cell)
{
	for(Cell &cell : data)
		cell = cell;
}
