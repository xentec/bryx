#include "map.h"

#include <sstream>


// Map
//########
Map::Map(u32 width, u32 height):
	width(width), height(height), data(width*height, Cell::EMPTY)
{}

void Map::clear(Cell new_cell)
{
	for(Cell &cell : data)
		cell = cell;
}

void Map::add(const Transistion &trn)
{
	trans.push_back(trn);
}

Cell Map::at(u32 x, u32 y) const
{
	return data[y*height+x];
}

Cell& Map::at(u32 x, u32 y)
{
	return data[y*height+x];
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
			str << (u8) (*this)[w][h] << " ";
		str << std::endl;
	}

	return str.str();
}

// Row
//########
Map::Row::Row(Map &map, usz offset):
	map(map), offset(offset)
{}

Cell& Map::Row::operator[](usz index)
{
	return map.data[offset+index];
}

