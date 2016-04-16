#include "player.h"

#include "game.h"

Player::Player(Game& game, Cell::Type type):
	game(game), color(type)
{}

Player& Player::operator =(const Player& other)
{
	color = other.color;
	game = other.game;
	return *this;
}

std::vector<Move> Player::possibleMoves()
{
	std::vector<Move> moves;

	for(i32 x = 0; x < game.map->width; x++)
	for(i32 y = 0; y < game.map->height; y++)
	{
		auto cellMoves = possibleMovesOn(game.map->at(x, y));
		moves.insert(moves.end(), cellMoves.begin(), cellMoves.end());
	}

	return moves;
}

std::vector<Move> Player::possibleMovesOn(Cell& cell)
{
	std::vector<Move> moves;

	for(u8 dir = Direction::N; dir < Direction::LAST; dir++)
	{
		Move move { *this, cell, (Direction) dir };
		Move::Error err = game.testMove(move);
		move.err = err;
		if(err == Move::Error::NONE)
			moves.push_back(move);
	}

	return moves;
}

Player& Player::choice() const
{
	// TODO: better choice algo
	return game.players[rand() % game.players.size()];
}

void Player::bonus()
{
	if(/* extremly complex calculation */ rand() % 2)
		bombs++;
	else
		overrides++;
}
