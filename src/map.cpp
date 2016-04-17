#include "map.h"

#include "util.h"

#include <cppformat/format.h>

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
		throw std::out_of_range(fmt::format("Direction {} invalid", input));
	return dir->second;
}


// Cell
//#######
Cell::Cell(Map& map, Type type):
	pos{-1,-1}, type(type), map(map), transitions()
{
	transitions.fill({nullptr, Direction::N});
}

Cell::Cell(const Cell &other):
	pos(other.pos), type(other.type), map(other.map), transitions(other.transitions)
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

string Cell::asString() const
{
	return fmt::format("{}:'{}'", pos, (char) type);
}


bool Cell::isFree() const
{
	switch(type)
	{
	case Type::EXPANSION:
	case Type::VOID:
		return false;
	default:
		return !isPlayer();
	}
	return true;
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


Cell* Cell::getNeighbor(Direction dir, bool with_trans) const
{
	const Transition& t = transitions[(usz)dir];
	if(with_trans && t.target) return t.target;

	Vec2 dirPos = pos + dir2vec(dir);
	return map.checkPos(dirPos) ? &map.at(dirPos) : nullptr;
}

void Cell::addTransistion(Direction in, Direction out, Cell* target)
{
	if(type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("adding transistion to void cell ({})", pos));

	Cell* wall = getNeighbor(in, false);
	if(wall && wall->type != Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion exit is not void ({}:{})", pos, dir2str(in)));

	if(!target || target->type == Cell::Type::VOID)
		throw std::runtime_error(fmt::format("transistion points to void cell ({})", target ? target->asString() : ""));

	transitions[(usz) in] = { target, (Direction)(((u32)out + 4) % 8) }; // reverse out direction
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
		str << fmt::format("{}\n", color::RESET) ;
	}

	return str.str();
}


// TODO: color und ansi flags
void Map::print(std::unordered_set<Vec2> highlight, bool colored, bool ansi) const
{
	fmt::print("   ");
	for (usz x = 0; x < width; x++)
	{
		if(x%2 == 0)
			fmt::print("{:^2}", x);
		else
			fmt::print("- ");
	}

	fmt::print("\n");

	for (usz y = 0; y < height; y++)
	{
		fmt::print("{}", color::RESET);
		if(y%2 == 0)
			fmt::print("{:2} ", y);
		else
			fmt::print(" | ");
		for (usz x = 0; x < width; x++)
		{
			ConsoleFormat color;
			const Cell& c = at(x, y);
			switch (c.type)
			{
			case Cell::Type::BONUS:     color = color::CYAN_LIGHT; break;
			case Cell::Type::CHOICE:    color = color::RED_LIGHT;  break;
			case Cell::Type::EMPTY:     color = color::GRAY_LIGHT; break;
			case Cell::Type::EXPANSION: color = color::CYAN_LIGHT; break;
			case Cell::Type::INVERSION: color = color::YELLOW;	   break;
			case Cell::Type::VOID:      color = color::GRAY;       break;
			default:
				if (c.isPlayer())
					color = color::YELLOW;
			}

			if(highlight.size() && highlight.find(c.pos) != highlight.end()) {
				color.color += 10;
				//color.attr = ConsoleFormat::BLINK;
			}

			fmt::print("{}{}{} ", color, (char)c.type, color::RESET);
		}
		fmt::print("\n");
	}
	fmt::print("{}", color::RESET);
}


bool Map::checkPos(const Vec2& pos) const
{
	return inBox(pos, Vec2::O, {(i32)width-1, (i32)height-1});
}
