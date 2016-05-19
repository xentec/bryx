#include "move.h"

#include "game.h"
#include "map.h"
#include "console.h"

Move::Move(Player& player, Cell* target):
	player(player),
	target(target), override(false),
	bonus(NONE), choice(Cell::Type::VOID),
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

	Format cf = color::YELLOW;
	std::unordered_map<vec, Format> hl;

	cf.setBG(Format::RED);
	hl.emplace(target->pos, cf);

	cf.setBG(Format::BLUE);

	for(Cell* c: captures)
		hl.emplace(c->pos, cf);

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
