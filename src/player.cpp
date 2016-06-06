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

string Player::asString() const
{
	fmt::MemoryWriter w;
	w << static_cast<char>(color);
	if(!name.empty())
		w.write(" \"{}\"", name);

	return w.str();
}

Player::~Player()
{}

std::list<Cell*> Player::stones()
{
	std::list<Cell*> s;
	for(Cell& c: game.getMap())
		if(c.type == color)
			s.push_back(&c);
	return s;
}

PossibleMoves Player::possibleMoves()
{
#if MOVES_ITERATOR
	return PossibleMoves { *this, game.getMap() };
#else
	std::deque<Move> moves;
	Move move { *this, nullptr };

	for(Cell& c: game.getMap())
	{
		move.target = &c;
		evaluate(move);

		if(move.err == Move::Error::NONE)
			moves.push_back(move);

		move.clear();
	}
	return moves;
#endif
}

void Player::evaluate(Move& move) const
{
	if(move.target->type == Cell::Type::VOID)
	{
		move.err = Move::Error::WRONG_START;
		return;
	}

	move.override = move.target->isCaptureable();
	if(move.override && overrides == 0)
	{
		move.err = Move::Error::NO_OVERRIDES;
		return;
	}

	Direction banned = Direction::_LAST;

	for(const Cell::Transition& dir : *move.target)
	{
		if(dir.entry == banned)
			continue;

		std::list<Cell*> line;
		const Cell::Transition* trn = &dir;

		while(trn->to && trn->to->isCaptureable())
		{
			if(trn->to->type == move.player.color)
			{
				if(!line.empty())
					move.captures.merge(line);
				break;
			}

			if(*move.target == *trn->to) // we're in a loop!
			{
				 // do not try the same dir you came from while looping
				banned = dir180(trn->entry);
				break;
			}

			line.push_back(trn->to);
			trn = &trn->to->getNeighbor(trn->entry);
		}
	}

	if(move.captures.empty() && move.target->type != Cell::Type::EXPANSION)
		move.err =  Move::Error::NO_CONNECTIONS;
	return;
}

#if MOVES_ITERATOR

usz PossibleMoves::size()
{
	usz i = 0;
	for(Move& m: *this)
		i++;

	return i;
}

bool PossibleMoves::empty()
{
	return size() == 0;
}

std::list<Move> PossibleMoves::all()
{
	std::deque<Move> all;
	for(Move& m: *this)
		all.push_back(m);

	return all;
}

PossibleMoves::iterator<Move> PossibleMoves::begin()
{
	return iterator<Move>(player, map.begin());
}

PossibleMoves::iterator<Move> PossibleMoves::end()
{
	return iterator<Move>(player, map.end());
}
#endif
