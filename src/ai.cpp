#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>
#include <numeric>
#include <map>

AI::AI(Game &game, Cell::Type color):
	Player(game, color, "bryx"),
	maxDepth(1), endTime()
{}

AI::AI(const AI &other):
	Player(other),
	maxDepth(other.maxDepth),
	endTime(other.endTime),
	evaltime(other.evaltime)
{}

AI::~AI()
{}

Player *AI::clone() const
{
	return new AI(*this);
}

static Quality
	infMin = std::numeric_limits<Quality>::min(),
	infMax = std::numeric_limits<Quality>::max();

inline bool minPrune (Quality h, Quality& a, Quality& v, Quality& b){ if(h < v) { v = h; if(v < b) b = v; } return v <= a; }; // min
inline bool maxPrune (Quality h, Quality& a, Quality& v, Quality& b){ if(h > v) { v = h; if(v > a) a = v; } return v >= b; }; // max


Move AI::move(PossibleMoves& posMoves, u32 time, u32 depth)
{
	maxDepth = depth;
	if(time)
		endTime = Clock::now() + Duration(time - time/100);

	Quality a = infMin, b = infMax;
	Move move = bestState(game, posMoves, 0, a, b).move;

	if(move.target)
	{
		switch(move.target->type)
		{
		case Cell::Type::BONUS:
			// TODO: better choice algo
			move.bonus = Move::Bonus::OVERRIDE;
			break;
		case Cell::Type::CHOICE:
			// TODO: better choice algo
			{
			  Player* c =
				*std::max_element(game.getPlayers().begin(), game.getPlayers().end(),
					[](Player* a, Player* b) { return a->stones().size() < b->stones().size();
				});
			  move.choice = c->color;
			}
			break;
		default:
			break;
		}
	}

	return move;
}


AIMove AI::bestState(Game& state, PossibleMoves& posMoves, u32 depth, Quality& a, Quality& b)
{
	Player& ply = state.currPlayer();

	if(posMoves.empty())
		throw std::runtime_error("no move");


	// TODO calc time
//	if(Clock::now()+game.aiData.evalTime*posMoves.size() > endTime)

	// sorting
	std::map<Quality, Move> sortedMoves;
	for(Move& m: posMoves)
	{
//		auto h = evalState(state, m);
//		moves.emplace(h, m);
		sortedMoves.emplace(evalState(state, m), m);
	}

	u32 d = depth+1;
	Quality v;

	std::function<bool(Quality h, Quality& a, Quality& v, Quality& b)> prune;
	std::vector<std::pair<const Quality, Move&> > moves;

	if(ply.color != color) // min
	{
		v = infMax;
		prune = minPrune;
		for(auto itr = sortedMoves.begin(); itr != sortedMoves.end(); itr++)
			moves.emplace_back(itr->first, itr->second);
	} else
	{
		v = infMin;
		prune = maxPrune;
		for(auto itr = sortedMoves.rbegin(); itr != sortedMoves.rend(); itr++)
			moves.emplace_back(itr->first, itr->second);
	}


	for(auto& m: moves)
	{
		println("MOVE POSSIBLE: {} ({})", m.second.target->pos, m.first);
		m.second.print();
		println();
	}
	AIMove next = { 0, { ply, nullptr }};

	if(d == maxDepth)
	{
		// reuse quality from BST
		for(auto& mp: moves)
		{
			if(ply.color != color ? mp.first < next.score : mp.first > next.score)
				next = { mp.first, mp.second };

			if(prune(mp.first, a, v, b))
				break;
		}
		if(next.move.target == nullptr)
			throw std::runtime_error("no move");
	} else
	{
		for(auto& mp: moves)
		{
			Move& m = mp.second;

			println("PLAYER: {} ({})\n", ply, depth);
			m.print();
			println();

			state.execute(m);
			Player& nextPly = state.nextPlayer();

			PossibleMoves nextPosMoves = nextPly.possibleMoves();
			if(nextPosMoves.empty())
			{
				next = { evalState(state, m), m };
			}
			else
			{
				println(" DEEPER ({}) ======>", d);
				next = bestState(state, nextPosMoves, d, a, b);
			}

			state.prevPlayer();
			state.undo();

			if(prune(next.score, a, v, b))
			{
				break;
			}
		}
		if(next.move.target == nullptr)
			throw std::runtime_error("no move");
	}

	return next;
}


#if 0
struct MoveNode
{
	Quality v;
	Move m;

//	std::map<Quality, MoveNode> children;
	std::deque<ptrU<MoveNode> > children;
};


Move AI::bestState2(Game &state)
{
	std::stack<std::map<Quality, Move> > stack;

	{
		// sorting
		std::map<Quality, Move> moves;
		for(Move& m: possibleMoves())
		{
			game.execute(m);
			Quality q = evalState(game);
			moves.emplace(q, m);
			game.undo();
		}
		stack.push(moves);
	}

	while(!stack.empty())
	{
		std::map<Quality, Move>& moveParents = stack.top();

		Player& ply = state.nextPlayer();

		// TODO calc time

		for(std::pair<Quality, Move> qm: moveParents)
		{
			std::map<Quality, Move> moveChildren;
			std::deque<Move> posMoves = ply.possibleMoves();

			// TODO calc time

			// sorting
			for(Move& m: posMoves)
			{
				game.execute(m);
				moveChildren.emplace(evalState(game), m);
				game.undo();
			}
			stack.push(moveChildren);
		}

<<<<<<< c35b790e20adcf2863efe385cfec88c9461bdc00
		moves.emplace(evalState(state, m), m);
=======
		state.prevPlayer();
>>>>>>> ai: cluster-commit!
	}
}
#endif


Quality AI::evalState(Game &state, Move& move) const
{
	state.execute(move);

	Quality h = 0;

	Player& futureMe = *state.getPlayers()[type2ply(color)];
//remove comment below if displeased with "new heuristik"
	/*
	h += 20 * futureMe.overrides;
	h += futureMe.bombs * state.defaults.bombsStrength;

	for(Cell& c: state.getMap())
	{
		if(c.type == color)
		{
			h = c.staticValue + 5;
			// #################

		} else if(c.isPlayer())
		{
			h = -c.staticValue - 5;
		}
	}
	*/
	//new heuristik
	for(Cell &c: state.getMap()){
		if(c.type == color)
			h += c.staticValue;
	}

//    h *= 1 + ((futureMe.possibleMoves() - possibleMoves()) / 100); //todo: (futureMe-currentMe)/100

	h += futureMe.overrides * game.aiData.expectedOverriteValue;
	h += futureMe.bombs * game.aiData.bombValue;

	state.undo();

	return h;
}
