#include "map.h"

#include "util/console.h"

#include <fmt/format.h>
#include <queue>
#include <unordered_map>

// Map
//########

#define HARD_CORNERS 0

#ifdef _WIN32
bool Map::printColored = false;
bool Map::printAnsi = false;
#else
bool Map::printColored = true;
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


std::vector<Cell*> Map::getQuad(vec centre, i32 radius)
{
	std::vector<Cell*> quad((radius*radius + radius)*4+1, nullptr);
	std::queue<Cell*> q;

	Cell& z = at(centre);
	q.push(&z);

	vec start = centre - vec(radius),
		end = centre + vec(radius);

	while(!q.empty())
	{
		Cell &c = *q.front();
		q.pop();

		c.helpValue++;
		quad.push_back(&c);

		for(Cell::Transition& tr : c)
		{
			if(tr.to == nullptr || tr.to->helpValue || !inBox(tr.to->pos, start, end))
				continue;

			if(tr.to && tr.to != c.getDirectNeighbor(tr.exit))
			{
				vec v = c.pos-centre;
				i32 r = radius-1-std::max(v.x, v.y);
				if(r >= 0)
				{
					auto sub = getQuad(tr.to->pos, r);
					quad.insert(quad.end(), sub.begin(), sub.end());
				}
			}
		}
	}

	for(Cell* c: quad)
		c->helpValue--;

#if SAFE_GUARDS
	check();
#endif
	quad.shrink_to_fit();

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

void Map::print(bool colored, bool ansi) const
{
	print(std::unordered_map<vec, console::Format>(), colored, ansi);
}


// TODO: color und ansi flags
void Map::print(std::unordered_map<vec, console::Format> highlight, bool colored, bool ansi) const
{
	using namespace console;
	if(quiet)
		return;

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


	for (u32 y = 0; y < height; y++)
	{
		if(y%5 == 0)
			fmt::print("{:2} ", y);
		else
			fmt::print(" | ");

		Format color;

		for (u32 x = 0; x < width; x++)
		{
			const Cell& c = at(x, y);
			string ch = c.asString();

			if(colored && printColored)
				color = c.getFormat();

			if(ansi && printAnsi)
			{
				switch (c.type)
				{
				case Cell::Type::BONUS:
				case Cell::Type::CHOICE:
				case Cell::Type::INVERSION: ch = "░"; break;
				case Cell::Type::EMPTY:     ch = "."; break;
				case Cell::Type::VOID:      ch = "█"; break;

				case Cell::Type::EXPANSION: ch = "x"; break;
				default: break;
				}
			}

			const auto& hl = highlight.find(c.pos);
			if(hl != highlight.cend())
				color = hl->second;

			fmt::print("{}{}{}{}", color, ch, color::RESET, space);
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

	// Evaluate cells
	//#################
	for(Cell& c: map)
	{
		if(c.type == Cell::Type::VOID)
			continue;

		u32 totalCounter = 0, maxCounter = 0, counter = 0;
		for(usz j = 0; j < (2 * Direction::_LAST); j++)       //checks for walls twice, to get the length of the wall
		{
			if(c.trans[j % Direction::_LAST].to == nullptr)   //which is contained in maxCount
			{
				counter++;
			}else
			{
				if(counter > maxCounter){
					maxCounter = counter;
				}
				totalCounter += counter;
				counter = 0;
			}
		}
		totalCounter /= 2;
		//maxCounter decides the maximum length of the wall
		//totalCounter decides the total amount of walls
		switch(maxCounter){
			case 0:                             //no attached walls
				c.staticValue = 1;
				break;
			case 1:                             //at least 1 wall with no neighbour-walls
				switch(totalCounter){
					case 2:                     //-||- with a total of 2 attached walls
						c.staticValue = 3;
						break;
					case 3:                     //-||- with a total of 3 attached walls
						c.staticValue = 4;
						break;
					case 4:                     //-||- with a total of 4 attached walls
						c.staticValue = 3;
						break;
					default:                    //exact 1 wall or unexpected amount of attached walls
						c.staticValue = 2;
						break;
				}
				break;
			case 2:                             //at least 2 neighbour-walls
				switch(totalCounter){
					case 3:                     //...
						c.staticValue = 4;
						break;
					case 4:
						c.staticValue = 5;
						break;
					default:
						c.staticValue = 3;
						break;
				}
				break;
			case 3:
				switch(totalCounter){
					case 4:
						c.staticValue = 6;
						break;
					case 5:
						c.staticValue = 4;
						break;
					case 6:
						c.staticValue = 3;
						break;
					default:
						c.staticValue = 5;
						break;
				}
				break;
			case 4:
				switch(totalCounter){
					case 5:
						c.staticValue = 8;
						break;
					case 6:
						c.staticValue = 6;
						break;
					case 7:
						c.staticValue = 4;
						break;
					default:
						c.staticValue = 10;
						break;
				}
				break;
			case 5:
				switch(totalCounter){
					case 6:
						c.staticValue = 6;
						break;
					default:
						c.staticValue = 7;
						break;
				}
				break;
			case 6:
				c.staticValue = 4;
				break;
			case 7:
				c.staticValue = 2;
				break;
			default:
				c.staticValue = 1;
				break;
		}
		if(c.type == Cell::Type::BONUS)
			c.staticValue += BONUS_VALUE;
		if(c.type == Cell::Type::CHOICE)
			c.staticValue += CHOICE_VALUE;
	}

	map.check();

	return map;
}
