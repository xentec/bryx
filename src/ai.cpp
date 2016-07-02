#include "ai.h"

#include "game.h"
#include "map.h"

#include <algorithm>
#include <numeric>
#include <map>

#define VERBOSE 0

static Quality
	infMin = std::numeric_limits<Quality>::min(),
	infMax = std::numeric_limits<Quality>::max();

bool AI::disableSorting = false;

AI::AI(Game &game, Cell::Type color):
	Player(game, color, "bryx"),
	maxDepth(1), endTime(Duration::max()), asp{infMin, infMax}
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




Move AI::move(PossibleMoves& posMoves, u32 time, u32 depth)
{
	maxDepth = 1;
	deepest = 0;
	states = 0;
	cutoffs = 0;

	Move move{ *this, nullptr };

#if SAFE_GUARDS
	string save = game.getMap().asString();
#endif

	println();
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


	moveChain.clear();

	Quality a = infMin, b = infMax;

	auto reset = (game.currPlayer().color == color) ?
				[](Quality& a, Quality& b) { a = infMin, b = infMax;}:
				[](Quality& a, Quality& b) { a = infMax, b = infMin;};

	if(!move.target)
	{
		if(time)
		{
			endTime = Clock::now() + Duration(time - time/20);
			TimePoint start, end;

			u32 deepest_pre = 0;

			do
			{
				start = Clock::now();

				moveChain.clear();

				println("Deepening START: {:<2} :: a: {:<5} b: {:<5} ", maxDepth, a, b);
				Quality v = bestState(game, posMoves, 0, a, b);
				//AIMove am = moveChain.front();
				//println("{} ({})", am.move, am.score);
				//am.move.print();

//				if(moveChain.size() < maxDepth-1)

				end = Clock::now();

				println("Deepening END: {:<2} States: {:<6} CO: {:<4} :: a: {:<5} v: {:<5} b: {:<5} ", maxDepth, states, cutoffs, a, v, b);
				fflush(stdout);

				if(deepest < maxDepth && deepest_pre == deepest)
				{
					println(":: End reached");
					break;
				}


				if(v <= a && b <= v)
				{
					println(console::color::RED_LIGHT, "BROKEN: a: {:<5} v: {:<5} b: {:<5}", a, v, b);

					reset(a, b);
					reset(asp.a, asp.b);
					continue;
				}


				a = v-(v-asp.a)/2;
				b = v+(v+asp.b)/2;

				deepest_pre = deepest;
				++maxDepth;

			} while(end + (end - start) * 2 < endTime);
		}
		else
		{
			maxDepth = depth;
			bestState(game, posMoves, 0, a, b);
		}
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


Move AI::bomb(u32 time)
{
#define BOMB_ENEMY_VALUE 2

	Cell* best = nullptr;
	Quality bestScore = infMin;

	for(Cell& c: game.getMap())
	{
		if(c.type == Cell::Type::VOID)
			continue;

		std::deque<Cell*> damage = game.getMap().getQuad(c.pos, game.defaults.bombsStrength);
		Quality score = 0;

		for(Cell* cp: damage)
		{
			if(!cp)
				score -= 2;

			Cell &c = *cp;

			if(c.type == color)
				score -= BOMB_ENEMY_VALUE-1;
			else if(c.isPlayer())
				score += BOMB_ENEMY_VALUE;
			else
				score -= 1;
		}

		if(score >= bestScore)
		{
			bestScore = score;
			best = &c;
		}
	}

	return { *this, best };
}




inline bool minPrune(const AIMove& q, Quality& a, AIMove& v, Quality& b)
{
#if VERBOSE
	println("a-b MIN :: q: {:<5} a: {:<5} v: {:<5} b: {:<5}", q.score, a, v.score, b);
#endif
	if(q.score < v.score)
	{
		v = q;
		if(v.score < b)
			b = v.score;
	}
#if VERBOSE
	print("a-b MIN :: q: {:<5} a: {:<5} v: {:<5} b: {:<5}  ", q.score, a, v.score, b);
	println("{}", v.score <= a ? "CUTOFF":"");
#endif
	return v.score <= a;
};

inline bool maxPrune(const AIMove& q, Quality& a, AIMove& v, Quality& b)
{
#if VERBOSE
	println("a-b MAX :: q: {:<5} a: {:<5} v: {:<5} b: {:<5}", q.score, a, v.score, b);
#endif
	if(q.score > v.score)
	{
		v = q;
		if(v.score > a)
			a = v.score;
	}
#if VERBOSE
	print("a-b MAX :: q: {:<5} a: {:<5} v: {:<5} b: {:<5}  ", q.score, a, v.score, b);
	println("{}", v.score >= b ? "CUTOFF":"");
#endif
	return v.score >= b;
}

inline bool isAfter(TimePoint endTime, Duration offset)
{
	return Clock::now()+offset > endTime;
}


Quality AI::bestState(Game& state, PossibleMoves& posMoves, u32 depth, Quality& a, Quality& b)
{
	Player& ply = state.currPlayer();

#if SAFE_GUARDS
	string save = state.getMap().asString();
#endif

	// sorting
	std::map<Quality, Move&> sortedMoves;
	std::vector<std::pair<const Quality, Move&> > moves;

	if(disableSorting)
	{
		for(Move& m: posMoves)
			moves.emplace_back(evalMove(state, m), m);
	}
	else
	{
		for(Move& m: posMoves)
			sortedMoves.emplace(evalMove(state, m), m);
	}

	u32 d = depth+1;

	std::function<bool(const AIMove& q, Quality& a, AIMove& v, Quality& b)> prune;
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

	if(d > deepest)
		deepest = d;

	if(d == maxDepth)
	{
		// reuse quality from BST
		for(auto& mp: moves)
		{
			if(prune({mp.first, mp.second}, a, best, b))
				break;
		}
#if VERBOSE > 1
		println("LIMIT END :: D: {}  Q: {} :: a: {:<5} b: {:<5} ", d, best.score, a, b);
#endif
	} else
	{
		Quality nextA = a, nextB = b;
		for(auto& mp: moves)
		{
			Move& m = mp.second;

#if VERBOSE > 3
			print("  MOVE POSSIBLE: {} :: ", mp.first);
			m.print();
			println();
#endif

			state.execute(m);

			AIMove next = { 0, { state.currPlayer(), nullptr }};
			PossibleMoves nextPosMoves = next.move.player.possibleMoves();
			if(nextPosMoves.empty())
			{
				next.score = mp.first + mp.first/3;
				println("GAME END :: D: {}  Q: {}", d, next.score);
			}
			else if(isAfter(endTime, game.aiData.evalTime*moves.size()))
			{
				next.score = mp.first - mp.first/5;
				println("NO TIME :: D: {}  Q: {}", d, next.score);
			}
			else
			{
#if VERBOSE
				println("DEEPER {} ==>", d);
				next.score = bestState(state, nextPosMoves, d, nextA, nextB);
				println(" <== {} ({}) ", d, next.score);
#else
				next.score = bestState(state, nextPosMoves, d, nextA, nextB);
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

#if SAFE_GUARDS
			if(save != state.getMap().asString())
			{
				println(" MAP CORRUPTION ");

				println("{}", save);
				println();
				println("{}", state.getMap().asString());


				throw std::runtime_error("map corruption");
			}
#endif

			if(prune({next.score, m}, a, best, b))
			{
				cutoffs++;
				break;
			}
		}
	}

#if SAFE_GUARDS
	if(save != state.getMap().asString())
		throw std::runtime_error("map corruption");
#endif

	asp.a = std::max(a, asp.a);
	asp.b = std::min(b, asp.b);

	moveChain.push_front(best);
	return best.score;
}

void AI::handleSpecials(Move& move) const
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

void AI::playerMoved(Move& real)
{
	if(!movePlan.empty())
	{
		Move& pred = movePlan.front().move;

		bool anticipated =
				   pred.target->pos == real.target->pos
				&& pred.bonus == real.bonus
				&& pred.choice == real.choice;

		if(anticipated)
		{
			println("ANTICIPATED");
			movePlan.pop_front();
			println("moveplan: {}", movePlan.size());
		} else
		{
			movePlan.clear();
			println("SURPRISED!");
		}
	}

	if(game.aiData.gameNearEnd == false)
	{
		if((game.aiData.amountMoves / 10) > (game.aiData.amountMoves - game.getMoveNum()))
		{
			for(Cell &c: game.getMap())
			{
				if(c.type == Cell::Type::VOID)
					continue;

				c.staticValue = 1;
				if(c.type == Cell::Type::BONUS)
					c.staticValue += BONUS_VALUE;
				else if(c.type == Cell::Type::CHOICE)
					c.staticValue += CHOICE_VALUE;
			}
			game.aiData.gameNearEnd = true;
		}
	}
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

	for(Cell &c: state.getMap()){
		if(c.type == color)
			h += c.staticValue;
	}

	Player& futureMe = *state.getPlayers()[type2ply(color)];

	h *= 10;

	h += futureMe.overrides * game.aiData.expectedOverriteValue;
	h += futureMe.bombs * game.aiData.bombValue;

	return h;
}

Quality AI::evalMove(Game &state, Move &move)
{

#if SAFE_GUARDS
	string save = game.getMap().asString();
#endif

	states++;
	handleSpecials(move);

	Player& futureMe = *state.getPlayers()[type2ply(color)];
	Quality h = 0;

	state.execute(move);

	h += evalState(state);
	h += futureMe.possibleMoves().size() / 3;

	if(move.override)
		h *= 2, h /= 3;

	state.undo();


#if SAFE_GUARDS
	if(save != game.getMap().asString())
		throw std::runtime_error("map corruption");
#endif

	return h;
}
