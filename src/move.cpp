#include "move.h"

Move::Move(Cell& start, Direction dir):
	start(start), end(nullptr), dir(dir)
{}

Move::Move(const Move& other):
	start(other.start), end(other.end), dir(other.dir),
	err(other.err),
	stones(other.stones)
{}

Move&Move::operator =(const Move& other)
{
	start = other.start;
	dir = other.dir;
	end = other.end;
	err = other.err;
	stones = other.stones;
	return *this;
}

std::string Move::err2str(Move::Error err)
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
