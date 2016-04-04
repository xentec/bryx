#include "map.h"

#include "util.h"

#include <cppformat/format.h>

// Map
//########
Map::Map(u32 width, u32 height):
	width(width), height(height), data(width*height, Cell::VOID)
{}

void Map::clear(Cell new_cell)
{
	for(Cell &cell : data)
		cell = new_cell;
}

void Map::add(const Transistion &trn)
{
	if(!isValid(trn.from))
		throw std::runtime_error("source location invalid");

	if(!isValid(trn.to))
		throw std::runtime_error("target location invalid");

	trans.push_back(trn);
}

std::vector<Transistion>& Map::getTransitstions()
{
	return trans;
}

Cell Map::at(u32 x, u32 y) const
{
	if(!checkXY(x,y))
		throw std::out_of_range(std::to_string(x)+":"+std::to_string(y));
	return data[x*height+y];
}

Cell& Map::at(u32 x, u32 y)
{
	if(!checkXY(x,y))
		throw std::out_of_range(std::to_string(x)+":"+std::to_string(y));
	return data[x*height+y];
}

Map::Row Map::operator[](usz index)
{
	return Row(*this, index*height);
}

string Map::asString(bool color)
{
	fmt::MemoryWriter str;
	for(usz y = 0; y < height; y++)
	{
		for(usz x = 0; x < width; x++)
		{			
			str << (u8) at(x,y) << ' ';
		}
		str << fmt::format("{}\n", color::RESET) ;
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
		return Cell::P1 <= cell && cell <= Cell::P8;
	}
}

bool Map::isValid(const Location& loc) const
{
	Vec2 dirPos = loc.pos + loc.dir;
	// is position on a playable cell...
	return at(loc.pos) != Cell::VOID
	//  AND  is the portal on '-' cell or maybe even outside the map?
		&& (!inBox(dirPos, Vec2::O, { (i32) width-1, (i32) height-1 }) || at(dirPos) == Cell::VOID);
}

bool Map::checkXY(u32 x, u32 y) const
{
	return x < width && y < height;
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

std::string Transistion::asString() const
{
	return from.asString() + " >-< " + to.asString();
}
