#pragma once

#include "global.h"
#include "vector.h"

#include <array>
#include <vector>
#include <unordered_map>


enum class Direction : u8
{
	N, NE, E, SE, S, SW, W, NW, LAST
};
Vec2 dir2vec(Direction dir);
string dir2str(Direction dir);
Direction str2dir(string input);

struct Map;
struct Cell
{
	enum class Type : char
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
	Cell& operator=(const Cell& other);

	bool operator ==(const Cell& other) const;
	bool operator !=(const Cell& other) const;

	Cell* getNeighbor(Direction dir, bool with_trans = true) const;
	void addTransistion(Direction in, Direction out, Cell* target);

	bool isFree() const;
	bool isPlayer() const;
	bool isCaptureable() const;

	string asString() const;

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
	void print(std::unordered_set<Vec2> highlight = std::unordered_set<Vec2>(), bool colored = true, bool ansi = true) const;

	const u32 width, height;
private:
	friend struct Cell;
	bool checkPos(const Vec2& pos) const;

	std::vector<Cell> data;
};
