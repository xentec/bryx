#include "move.h"

#include "game.h"
#include "map.h"
#include "util/console.h"

Move::Move(Player& player, Cell* target):
	player(player),
	target(target), override(false),
	bonus(Bonus::NONE), choice(Cell::Type::VOID),
	err(Error::NONE), captures()
{}

Move::Move(const Move& other):
	player(other.player),
	target(other.target), override(other.override),
	bonus(other.bonus), choice(other.choice),
	err(other.err), captures(other.captures)
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

bool Move::operator ==(const Move& other)
{
	if(!target || !other.target)
		return false;

	return player.color == other.player.color && *target == *other.target;
}

bool Move::operator !=(const Move& other)
{
	return !(*this == other);
}

string Move::asString() const
{
	return target ?
				fmt::format("P{} -> {}{}", player, target->pos, override ? "!":"") :
				fmt::format("P{} -> {}", player, "NULL");
}

void Move::print() const
{
	using namespace console;

	if(!target)
		return;

	Format cf;
	std::unordered_map<vec, Format> hl;

	cf.setBG(Cell::getTypeFormat(player.color).fg);
	cf.setFG(Format::BLACK);

	for(Cell* c: captures)
		hl.emplace(c->pos, cf);

//	cf.attr = Format::Attribute::BOLD;
	cf.setBG(Format::WHITE);
	hl.emplace(target->pos, cf);

	target->getMap().print(hl);
}

void Move::clear()
{
	captures.clear();

	target = nullptr;
	err = Error::NONE;
	choice = Cell::Type::VOID;
	bonus = Bonus::NONE;
}

string Move::err2str(Move::Error err)
{
	switch(err)
	{
	case Error::NO_CONNECTIONS: return "no straight lines to same stones";
	case Error::NO_OVERRIDES: return "missing overrides for this move";
	case Error::WRONG_START: return "target is not free or outside the map";
	default:
		return "none";
	}
}
