#include "move.h"

#include "game.h"
#include "map.h"
#include "consoleformat.h"

Move::Move(Player& player, Cell* target):
	player(player),
	target(target), override(false),
	err(Error::NONE), captures()
{}

Move& Move::operator =(const Move& other)
{
	player = other.player;
	target = other.target;
	override = other.override;
	err = other.err;
	captures = other.captures;
	return *this;
}

string Move::asString() const
{
	return target->asString();
}

void Move::print() const
{
	if(!target)
		return;

	ConsoleFormat c = color::YELLOW;
	std::unordered_map<const Cell*, ConsoleFormat> hl;

	c.setBG(ConsoleFormat::RED);
	hl.emplace(target, c);

	c.setBG(ConsoleFormat::BLUE);
	for(auto& list: captures)
	{
		auto begin = list.begin();
		for(; begin != list.end(); begin++)
			hl.emplace(*begin, c);
	}


	target->map.print(hl);
}

string Move::err2str(Move::Error err)
{
	switch(err)
	{
	case Error::NO_CONNECTIONS: return "no straight lines to same stones";
	case Error::NO_STONES_CAPTURED: return "no stones to capture";
	case Error::WRONG_START: return "target is not free or outside the map";
	default:
		return "none";
	}
}
