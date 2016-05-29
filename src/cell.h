#pragma once

#include "global.h"
#include "console.h"
#include "vector.h"

#include <array>

enum Direction : u8
{
	N, NE, E, SE, S, SW, W, NW, _LAST
};
string dir2str(Direction dir);
inline string dir2str(u32 i) { return dir2str((Direction) i); }
inline Direction dir180(Direction in) { return (Direction) ((in+Direction::_LAST/2) % (Direction::_LAST)); }

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
		Cell *to;
		Direction exit, entry;
	};


	Cell(Map &map, vec pos, Type type);
	Cell(Cell&& other) = default;

	Cell& operator =(const Cell& other);
	bool operator ==(const Cell& other) const;
	bool operator !=(const Cell& other) const;

	string asString() const;

	bool isFree() const;
	bool isPlayer() const;
	bool isCaptureable() const;
	bool isSpecial() const;

	void addTransition(Direction exit, Direction entry, Cell* target);

	Cell* getDirectNeighbor(Direction dir) const;
	Transition& getNeighbor(Direction dir);
	const Transition& getNeighbor(Direction dir) const;

	console::Format getFormat() const;

	static bool isValid(char ch);
	static console::Format getTypeFormat(Type type);


	const vec pos;
	Cell::Type type;
	std::array<Transition, 8> trans;

	i32 staticValue;

	Map& map;

//##############################
	template<class T, class C>
	struct iterator : std::iterator<std::random_access_iterator_tag, T>
	{
		typedef T   value_type;
		typedef T*  pointer;
		typedef T&  reference;
		typedef iterator<T,C> iter;

		reference operator*()  { return cell.getNeighbor((Direction)dir);  }
		pointer operator->()  { return &*this;  }

		iter& operator++()
		{
			dir++;
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
			dir = other.dir, cell = other.cell;
			return *this;
		}
		bool operator==(const iter& other) const
		{
			return dir == other.dir && cell == other.cell;
		}
		bool operator!=(const iter& other) const
		{
			return !(*this == other);
		}

		iterator(C& cell, Direction dir): dir(dir), cell(cell) {}
	private:
		usz dir;
		C& cell;
	};

	Cell::iterator<Transition, Cell> begin();
	Cell::iterator<Transition, Cell> end();

	Cell::iterator<const Transition, const Cell> begin() const;
	Cell::iterator<const Transition, const Cell> end() const;
};

inline u8 type2ply(Cell::Type type) { return type - Cell::Type::P1; }
inline Cell::Type ply2type(u8 ply_id) { return static_cast<Cell::Type>(ply_id + Cell::Type::P1); }
