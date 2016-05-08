#pragma once

#include "global.h"

#include "cell.h"
#include "consoleformat.h"
#include "vector.h"
#include "util.h"

#include <iterator>
#include <vector>
#include <unordered_map>

struct Map
{
	Map();
	Map(u32 width, u32 height);

	inline Cell& at(i32 x, i32 y) { return at({x, y}); }
	inline const Cell& at(i32 x, i32 y) const  { return at({x, y}); }

	Cell& at(const vec& pos);
	const Cell& at(const vec& pos) const;

	string asString();

	void print(bool colored = true, bool ansi = true) const;
	void print(std::unordered_map<const Cell*,ConsoleFormat> highlight, bool colored = true, bool ansi = true) const;

	const u32 width, height;
private:
	friend struct Cell;
	bool checkPos(const vec& pos) const;

	std::vector<Cell> data;

public:
	template<class T>
	struct iterator : std::iterator<std::random_access_iterator_tag, T>
	{
		typedef T   value_type;
		typedef T*  pointer;
		typedef T&  reference;
		typedef iterator<T> iter;

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

		iterator(Map&map, u32 x, u32 y): x(x), y(y), map(map) {}
	private:
		usz x, y;
		Map& map;
	};

	Map::iterator<Cell> begin();
	Map::iterator<Cell> end();

	Map::iterator<const Cell> cbegin();
	Map::iterator<const Cell> cend();
};
