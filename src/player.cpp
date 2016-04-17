#include "player.h"

#include "game.h"

Player::Player():
	id(-1), overrides(0), bombs(0), game(nullptr)
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

std::vector<Move> Player::possibleMoves()
{
	std::vector<Move> moves;

	for(i32 x = 0; x < game->map->width; x++)
	for(i32 y = 0; y < game->map->height; y++)
	{
		Cell& c = game->map->at(x, y);
		if(!c.isPlayer(id))
			continue;

		auto cellMoves = possibleMovesOn(c);
		moves.insert(moves.end(), cellMoves.begin(), cellMoves.end());
	}

	return moves;
}

std::vector<Move> Player::possibleMovesOn(Cell& cell)
{
	std::vector<Move> moves;

	for(u8 dir = Direction::N; dir < Direction::LAST; dir++)
	{
		Move move { *this, &cell, (Direction) dir };
		move.err = game->testMove(move);

		switch(move.err)
		{
		// add a override necessity with LINE_FULL but view it as a possible move
		case Move::Error::LINE_FULL: move.override = true; // hence the lack of break
		case Move::Error::NONE:	moves.push_back(move); break;
		default: break;
		}
	}

	return moves;
}
