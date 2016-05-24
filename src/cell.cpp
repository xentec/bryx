#include "cell.h"

#include "map.h"

#include <fmt/format.h>

// Cell
//#######
Cell::Cell(Map& map, vec pos, Type type):
	pos(pos), type(type), map(map)
{
	transitions.fill({nullptr, Direction::N});
}

Cell& Cell::operator =(const Cell &other)
{
	if(pos != other.pos)
		throw std::runtime_error("pos must be same");

	type = other.type;
	return *this;
}

bool Cell::operator ==(const Cell& other) const
{
	return pos == other.pos && type == other.type;
}

bool Cell::operator !=(const Cell& other) const
{
	return !(*this == other);
}

string Cell::asString() const
{
	return string(1, type);
}


bool Cell::isFree() const
{
	switch(type)
	{
	case Type::EMPTY:
	case Type::BONUS:
	case Type::CHOICE:
	case Type::INVERSION:
		return true;
	default:
		return false;
	}
}

bool Cell::isPlayer() const
{
	return Cell::Type::P1 <= type && type <= Cell::Type::P8;
}

bool Cell::isCaptureable() const
{
	return type == Type::EXPANSION || isPlayer();
}

bool Cell::isSpecial() const
{
	switch(type)
	{
	case Cell::Type::BONUS:
	case Cell::Type::CHOICE:
	case Cell::Type::INVERSION:
		return true;
	default:
		return false;
	}
}

Cell*Cell::getDirectNeighbor(Direction dir) const
{
	vec dirPos = pos + dir2vec(dir);
	return map.checkPos(dirPos) ? &map.at(dirPos) : nullptr;
}


Cell* Cell::getNeighbor(Direction& dir) const
{
	const Transition& t = transitions[(usz)dir];
	if(t.target)
	{
		dir = t.entry;
		return t.target;
	}
	Cell* c = getDirectNeighbor(dir);
	return c && c->type != Cell::Type::VOID ? c : nullptr;
}

void Cell::addTransistion(Direction exit, Direction entry, Cell* target)
{
	if(type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("adding transistion to void cell ({})", pos));

	Cell* wall = getDirectNeighbor(exit);
	if(wall && wall->type != Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion exit is not void ({}:{})", pos, dir2str(exit)));

	if(!target || target->type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion points to void cell ({})", target ? target->asString() : ""));

	transitions[exit] = { target, entry };
}

console::Format Cell::getFormat() const
{
	return getTypeFormat(type);
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

console::Format Cell::getTypeFormat(Cell::Type type)
{
	using namespace console;
	switch (type)
	{
		case Type::VOID:      return color::GRAY;       
		case Type::EMPTY:     return color::GRAY_LIGHT; 
	
		case Type::BONUS:     return color::YELLOW;     
		case Type::CHOICE:    return color::GREEN_LIGHT;
		case Type::INVERSION: return color::RED_LIGHT;  
	
		case Type::EXPANSION: return color::WHITE;      
	
		case Type::P1:        return color::RED;        
		case Type::P2:        return color::BLUE;       
		case Type::P3:        return color::GREEN;      
		case Type::P4:        return color::BROWN;      
		case Type::P5:        return color::MAGENTA;    
		case Type::P6:        return color::YELLOW;     
		case Type::P7:        return color::GRAY;       
		case Type::P8:        return color::CYAN;       
	}
}


// Direction
//############
vec dir2vec(Direction dir)
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

string dir2str(Direction dir)
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
