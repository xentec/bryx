#include "map.h"

#include "util/console.h"

#include <fmt/format.h>
#include <queue>
#include <unordered_map>

// Map
//########

#define HARD_CORNERS 0

#ifdef _WIN32
bool Map::printAnsi = false;
#else
bool Map::printAnsi = true;
#endif


Map::Map():
	Map(1,1)
{}

Map::Map(u32 width, u32 height):
	width(width), height(height)
{
	data.reserve(width*height); // valgrind hates this

	for(u32 x = 0; x < width; x++)
	for(u32 y = 0; y < height; y++)
		data.emplace_back(*this, vec{i32(x), i32(y)}, Cell::Type::VOID);
}

Map::Map(const Map& other):
	Map(other.width, other.height)
{
	for(u32 x = 0; x < width; x++)
	for(u32 y = 0; y < height; y++)
	{
		const Cell& o = other.at(x,y);
		Cell& c = at(x, y);

		c = o;
		for(u32 dir = Direction::N; dir < Direction::_LAST; dir++)
		{
			const Cell::Transition& trn = o.trans[dir];
			if(!trn.to)
				continue;

			c.trans[dir] = Cell::Transition{ &at(trn.to->pos), trn.exit, trn.entry };
		}
	}
}

Cell& Map::at(const vec &pos)
{
	if(!checkPos(pos))
		throw std::out_of_range("out of range: " + pos.asString());
	return data[pos.x * height + pos.y];
}

const Cell& Map::at(const vec &pos) const
{
	if(!checkPos(pos))
		throw std::out_of_range("out of range: " + pos.asString());
	return data[pos.x * height + pos.y];
}


std::list<Cell*> Map::getQuad(vec centre, i32 radius)
{
	struct Dist
	{
		Cell *c;
		i32 r;
	};

	std::queue<Dist> q;
	std::list<Cell*> quad;

	Cell& z = at(centre);
	q.push({ &z, radius });
	z.helpValue++;

	while(!q.empty())
	{
		Dist &dst = q.front();
		q.pop();

		quad.push_back(dst.c);

		if(dst.r == 0)
			continue;

		for(Cell::Transition& tr : *dst.c)
		{
			if(tr.to == nullptr || tr.to->helpValue)
				continue;

			q.push({ tr.to, dst.r-1 });
			tr.to->helpValue++;
		}
	}

	for(Cell* c: quad)
		c->helpValue--;

#if SAFE_GUARDS > 2
	check();
#endif

	return quad;
}


string Map::asString(bool transistions) const
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

void Map::print(bool print_statics) const
{
	print(std::unordered_map<vec, console::Format>(), print_statics);
}

void Map::print(std::unordered_map<vec, console::Format> highlight, bool print_statics) const
{
	using namespace console;
	if(quiet)
		return;

	// x axis
	auto printAxisNumber = printAnsi ?
				[](usz& x){ if(x%5 == 0) fmt::print("{:-<2}", x++); else fmt::print("-"); } :
				[](usz& x){ if(x%5 == 0) fmt::print("{:<2}", x); else fmt::print("- "); };

	u8 twice = print_statics;
	do
	{
		fmt::print("{:3}"," ");
		if(print_statics && !twice)
			fmt::print(" ");
		for (usz x = 0; x < width; x++)
			printAxisNumber(x);
	} while(twice--);

	fmt::print("\n");

	printAxisNumber = [](usz& y) { if(y%5 == 0)	fmt::print("{:2} ", y);	else fmt::print(" | "); };

	for (usz y = 0; y < height; y++)
	{
		// y axis
		printAxisNumber(y);

		// map v-line
		for (u32 x = 0; x < width; x++)
		{
			const Cell& c = at(x, y);
			string ch = c.asString();

			Format color = c.getFormat();

			if(printAnsi)
			{
				switch (c.type)
				{
				case Cell::Type::EMPTY:     ch = "."; break;
				case Cell::Type::VOID:      ch = "█"; break;

				case Cell::Type::EXPANSION: ch = "x"; break;
				default: break;
				}
			}

			const auto& hl = highlight.find(c.pos);
			if(hl != highlight.cend())
				color = hl->second;

			fmt::print("{}{}{}{}", color, ch, color::RESET, printAnsi ? "": " ");
		}
		if(print_statics)
		{
			fmt::print(" ");
			printAxisNumber(y);

			for (u32 x = 0; x < width; x++)
			{
				string ch = "░";
				Format color;

				const Cell& c = at(x, y);
				if(c.type == Cell::Type::VOID)
				{
					color = c.getFormat();

					ch = c.asString();
					if(printAnsi)
						ch = "█";
				}
				else
				{
					if(!c.isFree())
						ch = "#";
					if(c.staticValue > 1)
					{
						if(c.staticValue > 10)
							color = color::CYAN;
						else
							color = color::GREEN;
					}
					else if(c.staticValue == 1)
					{
						color = color::GRAY;
					}
					else
					{
						if(c.staticValue < -10)
							color = color::MAGENTA;
						else
							color = color::RED;
					}

					const auto& hl = highlight.find(c.pos);
						if(hl != highlight.cend())
							ch = "X";
				}
				fmt::print("{}{}{}{}", color, ch, color::RESET, printAnsi ? "": " ");
			}
		}
		fmt::print("\n");
	}
}

void Map::check() const
{
	for(const Cell& c: *this)
	{
		if(c.type == Cell::Type::VOID)
			continue;

		for(u32 dir = Direction::N; dir < Direction::_LAST; dir++)
		{
			const Cell::Transition& tr = c.trans[dir];

			if(tr.to != nullptr && tr.to->trans[dir180(tr.entry)].to != &c)
				throw std::runtime_error("map corruption");
		}
	}
}


bool Map::checkPos(const vec& pos) const
{
	return inBox(pos, {0,0}, {i32(width-1), i32(height-1)});
}

Map::iterator<Cell, Map> Map::begin()
{
	return Map::iterator<Cell, Map>{ *this, 0, 0 };
}

Map::iterator<Cell, Map> Map::end()
{
	return Map::iterator<Cell, Map>{ *this, width, 0 };
}

Map::iterator<const Cell, const Map> Map::begin() const
{
	return Map::iterator<const Cell, const Map>{ *this, 0, 0 };
}

Map::iterator<const Cell, const Map> Map::end() const
{
	return Map::iterator<const Cell, const Map>{ *this, width, 0 };
}

Map Map::load(std::istream& file)
{
	using std::stoi;

	// Map size
	std::vector<string> tmp = splitString(readline(file), ' ');
	if(tmp.size() < 2)
		throw std::runtime_error("map dimensions delimiter not found");

	Map map(stoi(tmp[1]), stoi(tmp[0]));

	for (usz y = 0; y < map.height; y++)
	{
		std::string line = readline(file);
		usz lm = (line.length()+1)/2;

		if(lm != map.width)
			throw std::runtime_error(fmt::format("length of line {} does not match map width: {} != {}", y+5, lm, map.width));

		for(usz x = 0; x < line.length(); x += 2) // ignore spaces inbetween
		{
			char c = line[x];
			if(!Cell::isValid(c))
				throw std::runtime_error(fmt::format("invalid cell character '{}' found at {}:{}", c, y+5, x));

			map.at(x/2, y).type = static_cast<Cell::Type>(c);
		}
	}

	// Transitions
	//#################

	// default ones
	for(Cell& c: map)
	{
		for(u32 d = Direction::N; d < Direction::_LAST; d++)
		{
			Direction dir = Direction(d);
#if HARD_CORNERS
			Cell *pre = c.getDirectNeighbor(Direction((d-1) % Direction::_LAST)),
				 *post = c.getDirectNeighbor(Direction((d+1) % Direction::_LAST));
			if(d % 2 && (!pre || !post || pre->type == Cell::Type::VOID || post->type == Cell::Type::VOID))
			{
				c.trans[d] = { nullptr, dir, dir };
			} else
			{
#endif
				Cell* nc = c.getDirectNeighbor(dir);
				c.trans[d] = { nc && c.type != Cell::Type::VOID && nc->type != Cell::Type::VOID ? nc : nullptr, dir, dir };
#if HARD_CORNERS
			}
#endif
		}
	}

	// special transistions

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
			from.addTransition(exit, dir180(entry), &to);
			to.addTransition(entry, dir180(exit), &from);
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

	map.check();

	return map;
}
