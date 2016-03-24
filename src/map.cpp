#include "map.h"

#include <sstream>


// Map
//########
Map::Map(u32 width, u32 height):
	width(width), height(height), data(width*height, Cell::VOID)
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
	return data[x*height+y];
}

Cell& Map::at(u32 x, u32 y)
{
	return data[x*height+y];
}

Map::Row Map::operator[](usz index)
{
	return Row(*this, index*height);
}

string Map::asString()
{
	std::stringstream str;
	for(usz y = 0; y < height; y++)
	{
		for(usz x = 0; x < width; x++)
			str << (u8) at(x,y) << " ";
		str << std::endl;
	}

	return str.str();
}

bool Map::isCell(u8 c)
{
	Cell cell = static_cast<Cell>(c);
	switch(cell)
	{
	case Cell::BONUS:
	case Cell::CHOICE:
	case Cell::EMPTY:
	case Cell::EXPANSION:
	case Cell::INVERSION:
	case Cell::VOID:
		return true;
	default:
		break;
	}

	if((u8)Cell::P1 <= c && c <= (u8)Cell::P8)
		return true;

	return false;
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

// Transistion
//##############
Transistion::Transistion(const Location& from, const Location& to):
	from(from), to(to)
{}
