#include "player.h"

Player::Player(Game& game, Cell::Type type):
	game(game), me(type)
{}

Player& Player::operator =(const Player& other)
{
	me = other.me;
	game = other.game;
	return *this;
}
