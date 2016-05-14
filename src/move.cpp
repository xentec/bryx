#include "move.h"

#include "game.h"
#include "map.h"
#include "console.h"

Move::Move(Player& player, Cell* target):
	player(player),
	target(target), override(false),
	bonus(NONE), choice(nullptr),
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
	using namespace console;

	if(!target)
		return;

	Format c = color::YELLOW;
	std::unordered_map<const Cell*, Format> hl;

	c.setBG(Format::RED);
	hl.emplace(target, c);

	c.setBG(Format::BLUE);
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
	case Error::WRONG_START: return "target is not free or outside the map";
	default:
		return "none";
	}
}
