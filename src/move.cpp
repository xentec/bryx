#include "move.h"

#include "game.h"
#include "map.h"
#include "consoleformat.h"

Move::Move(Player& player, Cell* target, bool override):
	player(player),
	target(target),
	err(Error::NONE),
	captures(), override(override)
{}

Move::Move(const Move& other):
	player(other.player),
	target(other.target),
	err(other.err),
	captures(other.captures), override(other.override)
{}

Move& Move::operator =(const Move& other)
{
	target = other.target;
	err = other.err;
	captures = other.captures;
	override = other.override;
	return *this;
}

u32 Move::score() const
{
	u32 score = 0;

	for(const std::list<Cell*>& line: captures)
	for(Cell* c: line)
	{
		switch(c->type)
		{
		case Cell::Type::BONUS: score += 10*(player.game->defaults.bombsStrength+5)/2; break;
		case Cell::Type::CHOICE: score += 30; break;
		case Cell::Type::INVERSION: score += 20; break;
		case Cell::Type::EXPANSION: score += 1; break;
		default:
			score += 2;
		}
	}
	if(override)
		score /= 2;

	return score;
}

string Move::asString() const
{
	//return fmt::format("{}:{} --> {}", start->pos, dir2str(dir), end ? end->pos : Vec2{-1,-1});
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
