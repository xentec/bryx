#include "location.h"

#include <unordered_map>

static Vec2 dir2vec(Direction dir);


Location::Location():
	Location({0,0}, Direction::N)
{}

Location::Location(Vec2 pos, Direction dir):
	Location(pos, dir2vec(dir))
{}

Location::Location(Vec2 pos, Vec2 dir):
	pos(pos), dir(dir)
{}

string Location::asString() const
{
	//static const char* dirs[] = {"⬆", "↗", "➡", "↘", "⬇", "↙", "⬅", "↖"};
	static const std::unordered_map<Vec2, const char*> dirs = {
		{{0,-1}, "N"},
		{{1,-1}, "NE"},	{{1, 0}, "E"},	{{1, 1}, "SE"},
		{{0, 1}, "S"},
		{{-1,1}, "SW"},	{{-1,0}, "W"}, 	{{-1,-1}, "NW"}
	};
	return pos.asString()+":"+dirs.at(dir);
}



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
