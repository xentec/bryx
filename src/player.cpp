#include "player.h"

#include "map.h"
#include "game.h"


Player::Player(const string& name):
	id(-1), name(name), overrides(0), bombs(0), game(nullptr)
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

std::vector<Move> Player::possibleMoves(bool override)
{
	std::vector<Move> moves;
	for(Cell& c: *game->map)
	{
		Move move { *this, &c, override };
		move.err = game->testMove(move);

		if(move.err == Move::Error::NONE)
			moves.push_back(move);
	}

	return moves;
}

u32 Player::score(bool inventory)
{
	u32 score = 0;
	if(inventory)
	{
		score += 2*bombs*game->defaults.bombsStrength;
		score += 5*overrides;
	}

	for(Move& m: possibleMoves())
	{
		score += m.score();
	}
	return score;
}
