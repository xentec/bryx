#include "player.h"

#include "map.h"
#include "game.h"


Player::Player(Game &game, const string& name):
	id(-1), name(name), overrides(0), bombs(0), game(game)
{}

Player& Player::operator =(const Player& other)
{
	id = other.id;
	overrides = other.overrides;
	bombs = other.bombs;
	return *this;
}

Player::~Player()
{}

std::vector<Cell *> Player::stones()
{
	std::vector<Cell*> s;
	for(Cell& c: game.map)
		s.push_back(&c);
	return s;
}

std::vector<Move> Player::possibleMoves()
{
	std::vector<Move> moves;
	for(Cell& c: game.map)
	{
		Move move { *this, &c };
		move.err = game.evaluate(move);

		if(move.err == Move::Error::NONE)
			moves.push_back(move);
	}

	return moves;
}

u32 Player::score()
{
	u32 score = 0;

	for(Cell& c: game.map)
		score += c.type == color;

	return score;
}
