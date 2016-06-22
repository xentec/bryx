#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>
#include <numeric>
#include <map>

#include <atomic>
#include <thread>

#include <pthread.h>

#define VERBOSE 0

AI::AI(Game &game, Cell::Type color):
	Player(game, color, "bryx"),
	maxDepth(1), endTime(Duration::max())
{}

AI::AI(const AI &other):
	Player(other),
	maxDepth(other.maxDepth),
	endTime(other.endTime),
	evaltime(other.evaltime)
{

}

AI::~AI()
{}

Player *AI::clone() const
{
	return new AI(*this);
}

static Quality
	infMin = std::numeric_limits<Quality>::min(),
	infMax = std::numeric_limits<Quality>::max();

inline bool minPrune (const AIMove& q, Quality& a, AIMove& v, Quality& b){ if(q.score < v.score) { v = q; if(v.score < b) b = v.score; } return v.score <= a; }; // min
inline bool maxPrune (const AIMove& q, Quality& a, AIMove& v, Quality& b){ if(q.score > v.score) { v = q; if(v.score > a) a = v.score; } return v.score >= b; }; // max


Move AI::move(PossibleMoves& posMoves, u32 time, u32 depth)
{
	maxDepth = 1;
	states = 0;

	Quality a = infMin, b = infMax;

	Move move{ *this, nullptr };

#if SAFE_GUARDS
	string save = game.getMap().asString();
#endif

	println("# CALCULATING MOVE");
	println("##########################");
	println();

	if(!movePlan.empty())
	{
#if SAFE_GUARDS
		if(movePlan.front().move.player.color != color)
			throw std::runtime_error("me != last player of the move plan");
#endif
		println("PREVIOUS MOVE PLAN:");
		for(AIMove& am: movePlan)
		{
			println("{} :: ", am.score);
			am.move.print();
			println();
			game.execute(am.move);
		}

		posMoves = game.currPlayer().possibleMoves();

		if(posMoves.empty())
			move = movePlan.front().move;
	}


	auto reset = (game.currPlayer().color == color) ?
				[](Quality& a, Quality& b) { a = infMin, b = infMax;}:
				[](Quality& a, Quality& b) { a = infMax, b = infMin;};

	if(!move.target)
	{
		if(time)
		{
			endTime = Clock::now() + Duration(time - time/200);
			TimePoint start, end;

			println();
			do
			{
				print("\rDeepening: {: <3} ", maxDepth);
				fflush(stdout);

				start = Clock::now();

				moveChain.clear();

				reset(a,b);

				bestState(game, posMoves, 0, a, b);
				//AIMove am = moveChain.front();
				//println("{} ({})", am.move, am.score);
				//am.move.print();

				if(moveChain.size() < maxDepth-1)
				{
					println("End reached");
					break;
				}

				++maxDepth;
				end = Clock::now();
			} while(end + (end - start) * 2 < endTime);

			println("Depth: {}", moveChain.size());
		}
		else
		{
			maxDepth = depth;
			maxDepth = 70;
			bestState(game, posMoves, 0, a, b);
		}
		println("States: {}", states);
	}

	// clean up
	for(usz i = 0; i < movePlan.size(); ++i)
		game.undo();

#if SAFE_GUARDS
		if(save != game.getMap().asString())
			throw std::runtime_error("map corruption");
#endif

	if(!move.target)
	{
#if VERBOSE
		println("MOVE PLAN ({}+{}):", moveChain.size(), movePlan.size());
		println("####################");

		usz steps = 0;
		for(AIMove am: movePlan)
		{
			print("{} :: ", am.score);
			am.move.print();
			game.execute(am.move);
			println();
			++steps;
		}

		println("## + #############");

		for(AIMove& am: moveChain)
		{
			print("{} :: ", am.score);
			am.move.print();
			if(am.move.override)
				print("");
			game.execute(am.move);
			println();
			++steps;
		}
		while(steps--)
		{
			game.undo();
		}

		println();
#endif

//		for(AIMove& am: moveChain)
//			movePlan.push_back(am);

		move = moveChain.front().move;

		moveChain.clear();

		println("##################################\n");
	}

#if SAFE_GUARDS
	if(move.player.color != color)
		throw std::runtime_error("player color shifter");

	if(save != game.getMap().asString())
		throw std::runtime_error("map corruption");
#endif

	handleSpecials(move);
	return move;
}


Quality AI::bestState(Game& state, PossibleMoves& posMoves, u32 depth, Quality& a, Quality& b)
{
	Player& ply = state.currPlayer();

	if(posMoves.empty())
		throw std::runtime_error("no move");

#if SAFE_GUARDS
	string save = state.getMap().asString();
#endif

	// sorting
	std::map<Quality, Move&> sortedMoves;
	for(Move& m: posMoves)
	{
		// TODO: better quality differenciation
		auto h = evalMove(state, m);
		sortedMoves.emplace(h, m);
//		sortedMoves.emplace(evalMove(state, m), m);
		states++;
		if(m.override)
			print("");
	}

	u32 d = depth+1;

	std::function<bool(const AIMove& q, Quality& a, AIMove& v, Quality& b)> prune;
	std::vector<std::pair<const Quality, Move&> > moves;

	AIMove best = { 0, { ply, nullptr }};


	if(ply.color != color) // min
	{
		best.score = infMax;
		prune = minPrune;
		for(auto itr = sortedMoves.begin(); itr != sortedMoves.end(); itr++)
			moves.emplace_back(itr->first, itr->second);
	} else
	{
		best.score = infMin;
		prune = maxPrune;
		for(auto itr = sortedMoves.rbegin(); itr != sortedMoves.rend(); itr++)
			moves.emplace_back(itr->first, itr->second);
	}

	if(d == maxDepth)
	{
		// reuse quality from BST
		for(auto& mp: moves)
		{
			if(prune({mp.first, mp.second}, a, best, b))
				break;
		}
		handleSpecials(best.move);

#if VERBOSE > 2
		println(" MAX ({}) {} :: {}", d, best.score, best.move);
#endif

	} else
	{
		for(auto& mp: moves)
		{
			Move& m = mp.second;

#if VERBOSE > 3
			print("  MOVE POSSIBLE: {} :: ", mp.first);
			m.print();
			println();
#endif

			if(m.override && m.player.color == Cell::Type::P1 && m.captures.empty())
				print("");

			handleSpecials(m);
			state.execute(m);

			AIMove next = { 0, { state.currPlayer(), nullptr }};
			PossibleMoves nextPosMoves = next.move.player.possibleMoves();
			if(nextPosMoves.empty())
			{
#if VERBOSE
				println(" END ({}) {} :: {}", d, mp.first, m);
#endif
				next.score = mp.first * 10;
			}
			else
			{
#if VERBOSE
				println("DEEPER {} ==>", d);
				next.score = bestState(state, nextPosMoves, d, a, b);
				println(" <== {} ({}) ", d, next.score);
#else
				next.score = bestState(state, nextPosMoves, d, a, b);
#endif
			}


#if SAFE_GUARDS
			if(state.getLastMove() != m)
				throw std::runtime_error("move stack corruption");
#endif

			state.undo();

#if VERBOSE > 3
			print("  MOVE UNDO: {} :: ", mp.first);
			m.print();
			println();
#endif

			if(prune({next.score+mp.first, m}, a, best, b))
			{
				break;
			}
		}
	}

#if SAFE_GUARDS
	if(save != state.getMap().asString())
		throw std::runtime_error("map corruption");
#endif

	if(best.move.override && best.move.player.color == Cell::Type::P1 && best.move.captures.empty())
		print("");

	moveChain.push_front(best);
	return best.score;
}

void AI::handleSpecials(Move& move)
{
	Game& game = move.player.game;

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

bool AI::playerMoved(Move& real)
{
	if(movePlan.empty())
		return false;

	Move& pred = movePlan.front().move;

	bool anticipated =
			   pred.target->pos == real.target->pos
			&& pred.bonus == real.bonus
			&& pred.choice == real.choice;

	if(anticipated)
	{
		movePlan.pop_front();
		println("moveplan: {}", movePlan.size());
	} else
		movePlan.clear();

	return anticipated;
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
		state.prevPlayer();
	}
}
#endif


Quality AI::evalState(Game &state) const
{
	Quality h = 0;

	Player& futureMe = *state.getPlayers()[type2ply(color)];
	for(Cell &c: state.getMap()){
		if(c.type == color)
			h += c.staticValue;
	}

	h += futureMe.overrides * game.aiData.expectedOverriteValue;
	h += futureMe.bombs * game.aiData.bombValue;


	return h;
}

Quality AI::evalMove(Game &state, Move &move) const
{
/*
	usz movesBefore = move.player.possibleMoves().size();
	state.execute(move);

	usz movesAfter = move.player.possibleMoves().size();
*/
	Quality h = evalState(state);
/*	h *= 1 + ((movesAfter - movesBefore) / 10); //todo: (futureMe-currentMe)/100

	state.undo();
*/
	if(move.override)
		h /= 2;

	return h;
}
