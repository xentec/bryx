#pragma once

#include "global.h"

#include "cell.h"
#include "move.h"

#include <fmt/ostream.h>
#include <deque>


#define MOVES_ITERATOR 0


struct Game;
#if MOVES_ITERATOR
struct PossibleMoves;
#else
using PossibleMoves = std::deque<Move>;
#endif


struct Player
{
	Player(Game& game, Cell::Type color, const string& name = "");
	Player(const Player& other);
	virtual ~Player();

	Player& operator =(const Player& other);
	virtual Player* clone() const = 0;

	string asString() const;

	virtual Move move(std::deque<Move>& moves, u32 time, u32 depth) = 0;

	std::list<Cell*> stones();
	PossibleMoves possibleMoves();

	void evaluate(Move& move) const;

	string name;

	Cell::Type color;
	u32 overrides;
	u32 bombs;

	Game& game;

	friend std::ostream& operator<<(std::ostream &os, const Player &object)
	{
		return os << object.asString();
	}
};


#if MOVES_ITERATOR
#include "map.h"

struct PossibleMoves
{
	Player& player;
	Map& map;

	usz size();
	bool empty();

	std::deque<Move> all();


	template<class T>
	struct iterator : std::iterator<std::forward_iterator_tag, T>
	{
		typedef T   value_type;
		typedef T*  pointer;
		typedef T&  reference;
		typedef iterator<T> iter;

		reference operator*() { return move; }
		pointer operator->()  { return &*this;  }

		iter& operator++()
		{
			while(mapIter < mapIter.getMap().end())
			{
				move.clear();

				move.target = &*mapIter++;
				move.player.evaluate(move);

				if(move.err == Move::Error::NONE)
					break;
			}

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
			move = other.move;
			mapIter = other.mapIter;
			return *this;
		}
		bool operator==(const iter& other) const
		{
			return mapIter == other.mapIter;
		}
		bool operator!=(const iter& other) const
		{
			return !(*this == other);
		}

		iterator(Player& player, Map::iterator<Cell, Map> pos):
			mapIter(pos), move{player, nullptr}
		{
			operator++();
		}
	private:
		Map::iterator<Cell, Map> mapIter;
		value_type move;
	};

	iterator<Move> begin();
	iterator<Move> end();

};
#endif
