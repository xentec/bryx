#include "player.h"

#include "map.h"
#include "game.h"


Player::Player(Game &game, u32 id, const string& name):
	id(id), name(name),
	overrides(0), bombs(0),
	game(game)
{}

Player& Player::operator =(const Player& other)
{
	id = other.id;
	name = other.name;
	overrides = other.overrides;
	bombs = other.bombs;
	return *this;
}

Player::~Player()
{}

std::vector<Cell *> Player::stones()
{
	std::vector<Cell*> s;
	for(Cell& c: game.getMap())
		if(c.type == color)
			s.push_back(&c);
	return s;
}

std::vector<Move> Player::possibleMoves()
{
	std::vector<Move> moves;
	for(Cell& c: game.getMap())
	{
		Move move { *this, &c };
		game.evaluate(move);

		if(move.err == Move::Error::NONE)
			moves.push_back(std::move(move));
	}

	return moves;
}

