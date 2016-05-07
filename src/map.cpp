#include "map.h"

#include "consoleformat.h"

#include <cppformat/format.h>

#include <unordered_map>

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
