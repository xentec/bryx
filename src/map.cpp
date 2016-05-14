#include "map.h"

#include "util.h"

#include <fmt/format.h>
#include <unordered_map>

// Map
//########
Map::Map():
	Map(1,1)
{}

Map::Map(u32 width, u32 height):
	width(width), height(height)
{
	data.reserve(width*height); // valgrind hates this

	for(i32 x = 0; x < width; x++)
	for(i32 y = 0; y < height; y++)
		data.emplace_back(*this, vec{x, y}, Cell::Type::VOID);
}

Map::Map(const Map& other):
	Map(other.width, other.height)
{
	for(i32 x = 0; x < width; x++)
	for(i32 y = 0; y < height; y++)
	{
		const Cell& o = other.at(x,y);
		Cell& c = at(x, y);

		c.type = o.type;
		for(u32 dir = Direction::N; dir < Direction::_LAST; dir++)
		{
			const Cell::Transition& trn = o.transitions[dir];
			if(!trn.target)
				continue;

			c.transitions[dir] = Cell::Transition{ &at(trn.target->pos), trn.entry };
		}
	}
}

Cell& Map::at(const vec &pos)
{
	if(!checkPos(pos))
		throw std::out_of_range(pos.asString());
	return data[pos.x * height + pos.y];
}

const Cell& Map::at(const vec &pos) const
{
	if(!checkPos(pos))
		throw std::out_of_range(pos.asString());
	return data[pos.x * height + pos.y];
}

string Map::asString(bool transistions)
{
	// TODO: Transistions

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
	std::unordered_map<const Cell*, console::Format> hl;
	print(hl, colored, ansi);
}


// TODO: color und ansi flags
void Map::print(std::unordered_map<const Cell*, console::Format> highlight, bool colored, bool ansi) const
{
	using namespace console;

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

		Format color;

		for (usz x = 0; x < width; x++)
		{
			const Cell& c = at(x, y);
			string ch = " ";

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
					ch[0] = (char) c.type;
				}
			}

			const auto& hl = highlight.find(&c);
			if(hl != highlight.cend())
				color = hl->second;

			fmt::print("{}{}{}{}", color, ch, color::RESET, space);
		}
		fmt::print("\n");
	}
}


bool Map::checkPos(const vec& pos) const
{
	return inBox(pos, {0,0}, {(i32)width-1, (i32)height-1});
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

Map Map::load(std::istream& file)
{
	using std::stoi;

	// Map size
	std::vector<string> tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("map dimensions delimiter not found");

	Map map(stoi(tmp[1]), stoi(tmp[0]));

	for (i32 y = 0; y < map.height; y++)
	{
		std::string line = readline(file);
		usz lm = (line.length()+1)/2;

		if(lm != map.width)
			throw std::runtime_error(fmt::format("length of line {} does not match map width: {} != {}", y+5, lm, map.width));

		for(i32 x = 0; x < line.length(); x += 2) // ignore spaces inbetween
		{
			char c = line[x];
			if(!Cell::isValid(c))
				throw std::runtime_error(fmt::format("invalid cell character '{}' found at {}:{}", c, y+5, x));

			map.at(x/2, y).type = static_cast<Cell::Type>(c);
		}
	}

	string line;
	while(readline(file, line))
	{
		if(line.empty()) continue;

		// Parse Transistion
		tmp = splitString(line, ' ');

		Cell &from = map.at(stoi(tmp.at(0)), stoi(tmp.at(1))),
			   &to = map.at(stoi(tmp.at(4)), stoi(tmp.at(5)));

		Direction exit = static_cast<Direction>(stoi(tmp.at(2))),
				 entry = static_cast<Direction>(stoi(tmp.at(6)));

		try {
			from.addTransistion(exit, dir180(entry), &to);
			to.addTransistion(entry, dir180(exit), &from);
		}
		catch(std::out_of_range& e)
		{
			fmt::print("error in transistion [{} <-> {}]: position is outside the map [{}]\n", from.asString(), to.asString(), e.what());
		}
		catch(std::exception& e)
		{
			fmt::print("error in transistion [{} <-> {}]: {}\n", from.asString(), to.asString(), e.what());
		}
	}

	return map;
}
