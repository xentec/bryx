#include "cell.h"

#include "map.h"

#include <unordered_map>

// Cell
//#######
Cell::Cell(Map& map, Vec2 pos, Type type):
	pos(pos), type(type), transitions(), map(map)
{
	transitions.fill({nullptr, Direction::N});
}

bool Cell::operator ==(const Cell& other) const
{
	return pos == other.pos && type == other.type;
}

bool Cell::operator !=(const Cell& other) const
{
	return !(*this == other);
}
/*
bool Cell::operator <(const Cell& other) const
{
	return pos.y < other.pos.y ? true : pos.x < other.pos.x;
}
*/

string Cell::asString() const
{
	return fmt::format("{}:'{}'", pos, (char) type);
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

bool Cell::isPlayer(i32 player) const
{
	return type == player + Cell::Type::P1;
}

void Cell::setPlayer(i32 player)
{
	if(player > 8)
		throw std::out_of_range("player index to high");
	type = (Cell::Type) (player + Cell::Type::P1);
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
	Vec2 dirPos = pos + dir2vec(dir);
	return map.checkPos(dirPos) ? &map.at(dirPos) : nullptr;
}


Cell* Cell::getNeighbor(Direction& dir) const
{
	const Transition& t = transitions[(usz)dir];
	if(t.target)
	{
		dir = t.out;
		return t.target;
	}
	Cell* c = getDirectNeighbor(dir);
	return c && c->type != Cell::Type::VOID ? c : nullptr;
}

void Cell::addTransistion(Direction in, Direction out, Cell* target)
{
	if(type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("adding transistion to void cell ({})", pos));

	Cell* wall = getDirectNeighbor(in);
	if(wall && wall->type != Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion exit is not void ({}:{})", pos, dir2str(in)));

	if(!target || target->type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion points to void cell ({})", target ? target->asString() : ""));

	transitions[(usz) in] = { target, dir180(out) }; // reverse out direction
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


// Direction
//############
Vec2 dir2vec(Direction dir)
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

Direction str2dir(std::string input)
{
	static std::unordered_map<string, Direction> tbl =
	{
		{"N", Direction::N},
		{"NE", Direction::NE},
		{"E", Direction::E},
		{"SE", Direction::SE},
		{"S", Direction::S},
		{"SW", Direction::SW},
		{"W", Direction::W},
		{"NW", Direction::NW},
	};

	auto dir = tbl.find(input);
	if(dir == tbl.end())
		throw std::runtime_error(fmt::format("Direction {} invalid", input));
	return dir->second;
}
