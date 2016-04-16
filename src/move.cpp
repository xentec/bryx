#include "move.h"

#include "game.h"

Move::Move(Player& player, Cell& start, Direction dir):
	player(player),
	start(start), end(nullptr), dir(dir),
	err(Error::NONE),
	stones(), override(false)
{}

Move::Move(const Move& other):
	player(other.player),
	start(other.start), end(other.end), dir(other.dir),
	err(other.err),
	stones(other.stones), override(other.override)
{}

Move& Move::operator =(const Move& other)
{
	start = other.start;
	dir = other.dir;
	end = other.end;
	err = other.err;
	stones = other.stones;
	override = other.override;
	return *this;
}

string Move::err2str(Move::Error err)
{
	switch(err)
	{
	case Error::NO_STONES_CAPTURED: return "no stones to capture";
	case Error::LINE_FULL: return "no free field in move line found";
	case Error::PATH_BLOCKED: return "path is not correct";
	case Error::WRONG_START: return "start field is not in possesion";
	default:
		return "none";
	}
}
