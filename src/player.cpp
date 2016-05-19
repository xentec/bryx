#include "player.h"

#include "map.h"
#include "game.h"


Player::Player(Game &game, Cell::Type color, const string& name):
	name(name),
	color(color), overrides(0), bombs(0),
	game(game)
{}

Player::Player(const Player &other):
	name(other.name),
	color(other.color), overrides(other.overrides), bombs(other.bombs),
	game(other.game)
{}

Player& Player::operator =(const Player& other)
{
	color = other.color;
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
		evaluate(move);

		if(move.err == Move::Error::NONE)
			moves.push_back(std::move(move));
	}

	return moves;
}

void Player::evaluate(Move& move) const
{
	move.err = Move::Error::NONE;
	if(move.target->type == Cell::Type::VOID)
	{
		move.err = Move::Error::WRONG_START;
		return;
	}

	move.override = move.target->isCaptureable();

	Direction banned = Direction::_LAST;
	Cell* cp = move.target;

	for(u32 dir = Direction::N; dir < Direction::_LAST; dir++)
	{
		Direction moveDir = (Direction) dir;
		if(dir == banned)
			continue;

		std::list<Cell*> line;
		Cell* cur = move.target->getNeighbor(moveDir);
		while(cur && cur->isCaptureable())
		{
			if(cur->type == move.player.color)
			{
				if(!line.empty())
					move.captures.push_back(line);
				break;
			}

			if(cp == cur) // we're in a loop!
			{
				 // do not try the same dir you came from while looping
				banned = dir180(moveDir);
				break;
			}

			line.push_back(cur);
			cur = cur->getNeighbor(moveDir);
		}
	}

	if(move.captures.empty())
		move.err =  Move::Error::NO_CONNECTIONS;
	return;
}

