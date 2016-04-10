#include "map.h"

#include "util.h"

#include <cppformat/format.h>

static Vec2 dir2vec(Direction dir)
{
	switch(dir)
	{
	case Direction::N:  return {0,-1};
	case Direction::NE: return {1,-1};
	case Direction::E:  return {1, 0};
	case Direction::SE: return {1, 1};
	case Direction::S:  return {0, 1};
	case Direction::SW: return {-1,1};
	case Direction::W:  return {-1,0};
	case Direction::NW: return {-1,-1};
	default:
		return {0,0};
	}
}

static string dir2str(Direction dir)
{
	switch(dir)
	{
	case Direction::N:  return "N";
	case Direction::NE: return "NE";
	case Direction::E:  return "E";
	case Direction::SE: return "SE";
	case Direction::S:  return "S";
	case Direction::SW: return "SW";
	case Direction::W:  return "W";
	case Direction::NW: return "NW";
	default:
		return "!";
	}
}

// Cell
//#######
Cell::Cell(Map& map, Vec2 pos, Type type):
	pos(pos), type(type), map(map)
{
	transistions.fill(nullptr);
}

Cell::Cell(const Cell &other):
	pos(other.pos), type(other.type), map(other.map)
{}

Cell &Cell::operator=(const Cell &other) // copy assignment
{
	type = other.type;
	std::copy(other.transistions.begin(), other.transistions.end(), transistions.data());

	return *this;
}

Cell &Cell::getNeighbor(Direction dir) const
{
	Cell* c = transistions[(usz)dir];
	return c ? *c : map.at(pos+dir2vec(dir));
}

void Cell::addTransistion(Direction dir, Cell* target)
{
	if(type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("adding transistion to void cell ({})", pos));

	if(map.checkPos(pos+dir2vec(dir)) && getNeighbor(dir).type != Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion exit is not void ({}:{})", pos, dir2str(dir)));

	transistions[(usz) dir] = target;
}

std::string Cell::asString() const
{
	return fmt::format("{}:'{}'", pos, (char) type);
}

bool Cell::isValid(char ch)
{
	Cell::Type ct = static_cast<Cell::Type>(ch);
	switch(ct)
	{
	case Cell::Type::BONUS:
	case Cell::Type::CHOICE:
	case Cell::Type::EMPTY:
	case Cell::Type::EXPANSION:
	case Cell::Type::INVERSION:
	case Cell::Type::VOID:
		return true;
	default:
		return Cell::Type::P1 <= ct && ct <= Cell::Type::P8;
	}
}

// Map
//########
Map::Map(u32 width, u32 height):
	width(width), height(height), data(width*height, Cell(*this, {-1, -1}, Cell::Type::VOID))
{
	Vec2 pos;
	for(i32 x = 0; x < width; x++)
	for(i32 y = 0; y < height; y++)
	{
		pos = { x, y };
		// dirty, but necessary const hack to set right position
		*const_cast<Vec2*>(&at(pos).pos) = pos;
	}

}

Map::~Map()
{
	clear();
}

void Map::clear(Cell::Type type)
{
	for(Cell& c: data)
		c.type = Cell::Type::VOID;
}

Cell& Map::at(const Vec2 &pos)
{
	if(!checkPos(pos))
		throw std::out_of_range(pos.asString());
	return data[pos.x * height + pos.y];
}

const Cell& Map::at(const Vec2 &pos) const
{
	if(!checkPos(pos))
		throw std::out_of_range(pos.asString());
	return data[pos.x * height + pos.y];
}
/*
std::unordered_multimap<Vec2, Vec2> Map::getTransitstions()
{
	std::unordered_multimap<Vec2, Vec2> res;
	for(auto& p: trans)
	{
		if(set.find(p.second) == set.end())
		{
			set.emplace(p.second, true);
			res.push_back(p.second);
		}
	}
	return res;
}
*/

string Map::asString()
{
	fmt::MemoryWriter str;
	for(usz y = 0; y < height; y++)
	{
		for(usz x = 0; x < width; x++)
		{
			str << (char) at(x,y).type << ' ';
		}
		str << fmt::format("{}\n", color::RESET) ;
	}

	return str.str();
}

bool Map::checkPos(const Vec2& pos) const
{
	return inBox(pos, Vec2::O, {(i32)width-1, (i32)height-1});
}

