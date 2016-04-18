#pragma once

#include "global.h"
#include "vector.h"

#include <iterator>

#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>


enum Direction : u8
{
	N, NE, E, SE, S, SW, W, NW, LAST
};
Vec2 dir2vec(Direction dir);
string dir2str(Direction dir);
Direction str2dir(string input);

struct Map;
struct Cell
{
	enum Type : u8
	{
		// standard fields
		VOID = '-',
		EMPTY = '0',

		// special
		BONUS = 'b',
		CHOICE = 'c',
		INVERSION = 'i',
		EXPANSION = 'x',

		// players
		P1 = '1', P2, P3, P4, P5, P6, P7, P8,
	};
	struct Transition
	{
		Cell* target;
		Direction out;
	};


	Cell(Map &map, Type type);
	Cell(const Cell& other);
	~Cell();

	Cell& operator =(const Cell& other);
	bool operator ==(const Cell& other) const;
	bool operator !=(const Cell& other) const;

	string asString() const;

	bool isFree() const;
	bool isPlayer() const;
	bool isPlayer(i32 player) const;
	void setPlayer(i32 player);
	bool isCaptureable() const;

	Cell* getDirectNeighbor(Direction dir) const;
	Cell* getNeighbor(Direction& dir) const;
	void addTransistion(Direction in, Direction out, Cell* target);

	static bool isValid(char ch);

	const Vec2 pos;
	Cell::Type type;
	std::array<Transition, 8> transitions;

	Map& map;
};

struct Map
{
	Map(u32 width, u32 height);

	void clear(Cell::Type type = Cell::Type::VOID);

	inline Cell& at(i32 x, i32 y) { return at({x, y}); }
	inline const Cell& at(i32 x, i32 y) const  { return at({x, y}); }

	Cell& at(const Vec2& pos);
	const Cell& at(const Vec2& pos) const;

	string asString();
	void print(std::unordered_set<Cell*> highlight = std::unordered_set<Cell*>(), bool colored = true, bool ansi = true) const;

	const u32 width, height;
private:
	friend struct Cell;
	bool checkPos(const Vec2& pos) const;

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
			if(++x == map.width)
				x = 0, ++y;
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



