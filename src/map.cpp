#include "map.h"

#include "consoleformat.h"

#include <cppformat/format.h>

#include <unordered_map>

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


// Cell
//#######
Cell::Cell(Map& map, Type type):
	pos{-1,-1}, type(type), transitions(), map(map)
{
	transitions.fill({nullptr, Direction::N});
}

Cell::Cell(const Cell &other):
	pos(other.pos), type(other.type), transitions(other.transitions), map(other.map)
{}

Cell::~Cell()
{
	// for break points
}

Cell &Cell::operator=(const Cell &other)
{
	type = other.type;
	transitions = other.transitions;
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
		break;
	}
	return false;
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
	return getDirectNeighbor(dir);
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

	transitions[(usz) in] = { target, (Direction)((out + 4) % 8) }; // reverse out direction
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
	width(width), height(height), data(width*height, Cell(*this, Cell::Type::VOID))
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

string Map::asString()
{
	fmt::MemoryWriter str;
	for(usz y = 0; y < height; y++)
	{
		for(usz x = 0; x < width; x++)
		{
			str << (char) at(x,y).type << ' ';
		}
		str << "\n";
	}

	return str.str();
}

void Map::print(bool colored, bool ansi) const
{
	std::unordered_map<const Cell*, ConsoleFormat> hl;
	print(hl, colored, ansi);
}


// TODO: color und ansi flags
void Map::print(std::unordered_map<const Cell*,ConsoleFormat> highlight, bool colored, bool ansi) const
{
	string space = ansi ? "": " ";

	fmt::print("   ");
	for (usz x = 0; x < width; x++)
	{
		if(x%5 == 0)
			fmt::print("{:-<2}", x++);
		else
			fmt::print("-{}", space);
	}

	fmt::print("\n");


	for (usz y = 0; y < height; y++)
	{
		if(y%5 == 0)
			fmt::print("{:2} ", y);
		else
			fmt::print(" | ");

		for (usz x = 0; x < width; x++)
		{
			const Cell& c = at(x, y);

			ConsoleFormat color;
			string ch;

			if(colored)
			{
				switch (c.type)
				{
				case Cell::Type::VOID:      color = color::GRAY;       break;
				case Cell::Type::EMPTY:     color = color::GRAY_LIGHT; break;

				case Cell::Type::BONUS:     color = color::YELLOW;     break;
				case Cell::Type::CHOICE:    color = color::GREEN_LIGHT;break;
				case Cell::Type::INVERSION: color = color::RED_LIGHT;  break;

				case Cell::Type::EXPANSION: color = color::WHITE;      break;

				case Cell::Type::P1:        color = color::RED;        break;
				case Cell::Type::P2:        color = color::BLUE;       break;
				case Cell::Type::P3:        color = color::GREEN;      break;
				case Cell::Type::P4:        color = color::BROWN;      break;
				case Cell::Type::P5:        color = color::MAGENTA;    break;
				case Cell::Type::P6:        color = color::YELLOW;     break;
				case Cell::Type::P7:        color = color::GRAY;       break;
				case Cell::Type::P8:        color = color::CYAN;       break;
				default: break;
				}
			}

			if(ansi)
			{
				switch (c.type)
				{
				case Cell::Type::BONUS:
				case Cell::Type::CHOICE:
				case Cell::Type::INVERSION: ch = "░"; break;
				case Cell::Type::EMPTY:     ch = " "; break;
				case Cell::Type::VOID:      ch = "█"; break;

				case Cell::Type::EXPANSION: ch = "x"; break;

				default:
					ch = (const char*) &c.type;
				}
			}

			if(!highlight.empty())
			{
				const auto& hl = highlight.find(&c);
				if(hl != highlight.cend())
					color = hl->second;
			}
			fmt::print("{}{}{}{}", color, ch, color::RESET, space);
		}
		fmt::print("\n");
	}
}


bool Map::checkPos(const Vec2& pos) const
{
	return inBox(pos, Vec2::O, {(i32)width-1, (i32)height-1});
}

Map::iterator<Cell> Map::begin()
{
	return Map::iterator<Cell>{ *this, 0, 0 };
}

Map::iterator<Cell> Map::end()
{
	return Map::iterator<Cell>{ *this, width-1, height-1 };
}

Map::iterator<const Cell> Map::cbegin()
{
	return Map::iterator<const Cell>{ *this, 0, 0 };
}

Map::iterator<const Cell> Map::cend()
{
	return Map::iterator<const Cell>{ *this, width-1, height-1 };
}
