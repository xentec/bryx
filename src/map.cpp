#include "map.h"

#include <sstream>

Map::Map(u32 width, u32 height):
	width(width), height(height), data(width*height, Cell::EMPTY)
{}

void Map::clear(Cell new_cell)
{
	for(Cell &cell : data)
		cell = cell;
}

Map::Row Map::operator[](usz index)
{
	return Row(*this, index*height);
}

string Map::asString()
{
	std::stringstream str;
	for(usz h = 0; h < height; h++)
	{
		for(usz w = 0; w < width; w++)
			str << (char) (*this)[w][h] << " ";
		str << std::endl;
	}

	return str.str();
}



// Row

Map::Row::Row(Map &map, usz offset):
	map(map), offset(offset)
{}

Cell& Map::Row::operator[](usz index)
{
	return map.data[offset+index];
}

