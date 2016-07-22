#pragma once

#include "global.h"

#include "cell.h"
#include "vector.h"
#include "util/console.h"
#include "util/misc.h"

#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>

#define BONUS_VALUE 11
#define CHOICE_VALUE 5

struct Map
{
	static bool printAnsi;
	static bool printStatic;


	Map();
	Map(u32 width, u32 height);
	Map(const Map& other);

	inline Cell& at(i32 x, i32 y) { return at({x, y}); }
	inline const Cell& at(i32 x, i32 y) const  { return at({x, y}); }

	Cell& at(const vec& pos);
	const Cell& at(const vec& pos) const;

	std::list<Cell*> getQuad(vec centre, i32 radius);

	string asString(bool transistions = true) const;

	void print(bool print_statics = printStatic) const;
	void print(std::unordered_map<vec, console::Format> highlight, bool print_statics = printStatic) const;

	void check() const;

	static Map load(std::istream& file);

	const u32 width, height;
private:
	friend struct Cell;
	bool checkPos(const vec& pos) const;

	std::vector<Cell> data;

public:
	template<class T, class M>
	struct iterator : std::iterator<std::random_access_iterator_tag, T>
	{
		typedef T   value_type;
		typedef T*  pointer;
		typedef T&  reference;
		typedef iterator<T,M> iter;

		reference operator*()  { return map.at(x,y);  }
		pointer operator->()   { return &map.at(x,y); }

		iter& operator++()
		{
			if(++y == map.height)
				y = 0, ++x;
			return *this;
		}
		iter operator++(int)
		{
			iter pre = *this;
			++(*this);
			return pre;
		}

		iter& operator=(const iter& other)
		{
			x = other.x, y = other.y, map = other.map;
			return *this;
		}
		bool operator==(const iter& other) const
		{
			return x == other.x && y == other.y && &map == &other.map;
		}
		bool operator!=(const iter& other) const
		{
			return !(*this == other);
		}
		bool operator<(const iter& other) const
		{
			return x < other.x || (x == other.x && y < other.y);
		}

		inline M& getMap() { return map; }

		iterator(M &map, u32 x, u32 y): x(x), y(y), map(map) {}
	private:
		usz x, y;
		M &map;
	};

	Map::iterator<Cell,Map> begin();
	Map::iterator<Cell,Map> end();

	Map::iterator<const Cell, const Map> begin() const;
	Map::iterator<const Cell, const Map> end() const;
};
