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
	maxDepth(1), endTime(Duration::max()),
	stopSearch(false),
	asp{infMin, infMax},
	stats{}
{
	stats.durs.escape.prev.data.fill(Duration(0));
	stats.durs.eval.prev.data.fill(game.aiData.evalTime*4);
}

AI::AI(const AI &other):
	Player(other),
	maxDepth(other.maxDepth),
	endTime(other.endTime),
	stopSearch(other.stopSearch),
	stats(other.stats)
{}

AI::~AI()
{}

Player *AI::clone() const
{
	return new AI(*this);
}


Move AI::move(PossibleMoves& posMoves, u32 time, u32 depth)
{
	maxDepth = 1;

	Move move{ *this, nullptr };

#if SAFE_GUARDS > 2
	string save = game.getMap().asString();
#endif

	if(!movePlan.empty())
	{
#if SAFE_GUARDS
		if(movePlan.front().move.getPlayer().color != color)
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

	Quality a, b;

	auto reset = (game.currPlayer().color == color) ?
				[](Quality& a, Quality& b) { a = infMin, b = infMax;}:
				[](Quality& a, Quality& b) { a = infMax, b = infMin;};


	reset(a, b);
	reset(asp.a, asp.b);

	if(!move.target)
	{
		if(time)
		{
			stopSearch = false;
			stats.states = 0;
			endTime = Clock::now() + Duration(time - time/5);

			TimePoint start, end;
			u32 deepest_pre = 0;

			do
			{
				start = Clock::now();

				stats.cutoffs = stats.gameEnds = 0;
				moveChain.clear();

				println("Deepening START: {:<2}", maxDepth);
				println("  a: {}", a);
				println("  b: {}", b);
				Quality v = bestState(game, posMoves, 0, a, b);
				//AIMove am = moveChain.front();
				//println("{} ({})", am.move, am.score);
				//am.move.print();

//				if(moveChain.size() < maxDepth-1)

				println("Deepening END: {:<2}", maxDepth);
				println("  States: {}, Cutoffs: {}, GameEnds: {}", stats.states, stats.cutoffs, stats.gameEnds);
				println("  a: {:<8}  max a: {}", a, asp.a);
				println(console::color::GREEN_LIGHT, "  v: {}", v);
				println("  b: {:<8}  max b: {}", b, asp.b);

				if(stopSearch)
					break;

				if(stats.deepest < maxDepth && deepest_pre == stats.deepest)
				{
					println(console::color::GREEN, "  Max possible depth reached!");
					break;
				}

				if(v <= a && b <= v)
				{
					println(console::color::RED_LIGHT, "  AW to narrow!");
					println();

					reset(a, b);
					reset(asp.a, asp.b);
					continue;
				}
				println();
				fflush(stdout);

				a = v-(v+asp.a)/2;
				b = v+(v+asp.b)/2;

				deepest_pre = stats.deepest;
				++maxDepth;

				end = Clock::now();
			} while(end + (end - start) < endTime);

			println("  avr time for eval:   ~{} ms", stats.durs.eval.prev.average().count());
			println("  avr time for escape: ~{} ms", stats.durs.escape.prev.average().count());
		}
		else
		{
			maxDepth = depth;
			println("Searching to depth {}", maxDepth);

			bestState(game, posMoves, 0, a, b);

			println(console::color::GREEN, "Done");
			println("  States: {}, Cutoffs: {}", stats.states, stats.cutoffs);
		}
		println();
	}


	// clean up
	for(usz i = 0; i < movePlan.size(); ++i)
		game.undo();

#if SAFE_GUARDS > 2
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

		for(AIMove& am: moveChain)
			movePlan.push_back(am);
#endif

		move = moveChain.front().move;
	}

#if SAFE_GUARDS
	if(move.getPlayer().color != color)
		throw std::runtime_error("player color shifter");
#if SAFE_GUARDS > 2
	if(save != game.getMap().asString())
		throw std::runtime_error("map corruption");
#endif
#endif

//	if(stopSearch)
//		stats.durs.escape.stop();

	return move;
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


// Pruner

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


/*

	 ######  ########    ###    ########   ######  ##     ##
	##    ## ##         ## ##   ##     ## ##    ## ##     ##
	##       ##        ##   ##  ##     ## ##       ##     ##
	 ######  ######   ##     ## ########  ##       #########
		  ## ##       ######### ##   ##   ##       ##     ##
	##    ## ##       ##     ## ##    ##  ##    ## ##     ##
	 ######  ######## ##     ## ##     ##  ######  ##     ##

*/

Quality AI::bestState(Game& state, PossibleMoves& posMoves, u32 depth, Quality& a, Quality& b)
{
	Player& ply = state.currPlayer();

#if SAFE_GUARDS > 2
	string save = state.getMap().asString();
#endif

	// sorting
	std::map<Quality, Move&> sortedMoves;
	std::vector<std::pair<const Quality, Move&> > moves;

	std::function<void(Quality q, Move& m)> insert = [&](Quality q, Move& m) { sortedMoves.emplace(q, m); };
	if(disableSorting)
		insert = [&](Quality q, Move& m) { moves.emplace_back(q, m); };

	u32 d = depth+1;

	TimePoint end;

	for(Move& m: posMoves)
	{
		insert(evalMove(state, m), m);
		if((end = Clock::now() + stats.durs.eval.prev.data.back()) > endTime)
		{
			d = maxDepth;
			stopSearch = true;
			println(console::color::YELLOW, "NO TIME! (eval) D: {}: Over {} ms", depth, (end-Clock::now()).count());
			println("  moves: {}", posMoves.size());
			println("  evaluated: {}", disableSorting ? moves.size() : sortedMoves.size());
			break;
		}
	}


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

	if(d == maxDepth)
	{
		if(d > stats.deepest)
			stats.deepest = d;

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

			if(stopSearch)
				break;

#if VERBOSE > 3
			print("  MOVE POSSIBLE: {} :: ", mp.first);
			m.print();
			println();
#endif

			state.execute(m);

			AIMove next = { 0, { state.currPlayer(), nullptr }};
			PossibleMoves nextPosMoves = next.move.getPlayer().possibleMoves();
			Duration eval, esc;

			if(nextPosMoves.empty())
			{
				next.score = mp.first + mp.first/3;
				stats.gameEnds++;
			}
			else if((end = Clock::now() + (eval = stats.durs.eval.prev.average() * nextPosMoves.size()) + (esc = stats.durs.escape.prev.average())) > endTime)
			{
				//stats.durs.escape.start();
				stopSearch = true;

				next.score = mp.first - mp.first/5;
				println(console::color::YELLOW, "NO TIME! (next) D: {}: Over {} ms", depth, (end-Clock::now()).count());
				println("  time for next depth: ~{} ms", eval.count());
				println("  time for escape:     ~{} ms", esc.count());
				println();
			}
			else
			{
				next.score = bestState(state, nextPosMoves, d, nextA, nextB);
			}

#if SAFE_GUARDS > 1
			if(state.getLastMove() != m)
				throw std::runtime_error("move stack corruption");
#endif

			state.undo();

#if VERBOSE > 3
			print("  MOVE UNDO: {} :: ", mp.first);
			m.print();
			println();
#endif

#if SAFE_GUARDS > 2
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
				stats.cutoffs++;
				break;
			}
		}
	}

#if SAFE_GUARDS > 2
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
	Game& game = move.getPlayer().game;

	switch(move.target->type)
	{
	case Cell::Type::BONUS:
		// TODO: better choice algo
		move.bonus = Move::Bonus::OVERRIDE;
		break;
	case Cell::Type::CHOICE:
		// TODO: better choice algo
		{
			std::vector<u32> scores(game.defaults.players, 0);

			for(Cell& c: game.getMap())
			{
				if(c.isPlayer())
					++scores[type2ply(c.type)];
			}

			u32 best = 0;
			for(usz i = 0; i < game.defaults.players; i++)
			{
				if(scores[i] > best)
				{
					best = scores[i];
					move.choice = ply2type(i);
				}
			}

		}
		break;
	default:
		break;
	}
}


Move AI::bomb(u32 time)
{
#define BOMB_ENEMY_VALUE 2
	endTime = Clock::now() + Duration(time - time/20);

	println("Locating target...");

	Move best{ *this, nullptr };
	Quality bestScore = infMin;

	for(Cell& c: game.getMap())
	{
		if(c.type == Cell::Type::VOID)
			continue;

		std::list<Cell*> damage = game.getMap().getQuad(c.pos, game.defaults.bombsStrength);
		u32 me, enemy, empty;
		me = enemy = empty = 0;

		for(Cell* c: damage)
		{
			bool m = c->type == color,
				 p = c->isPlayer();

			me += m;
			enemy += p && !m;
			empty += !p;
		}

		i32 score = enemy*2 - empty - me*3;

		if(score >= bestScore)
		{
			bestScore = score;
			best.target = &c;
			best.captures.assign(damage.begin(), damage.end());
		}
		if(time && Clock::now() > endTime)
		{
			println(console::color::YELLOW, "NO TIME!");
			break;
		}
	}

	return best;
}

/*

######## ##     ##    ###    ##
##       ##     ##   ## ##   ##
##       ##     ##  ##   ##  ##
######   ##     ## ##     ## ##
##        ##   ##  ######### ##
##         ## ##   ##     ## ##
########    ###    ##     ## ########

*/
Quality AI::evalState(Game &state) const
{
	Quality h = 0;

	std::vector<u32> scores(game.defaults.players, 0);

	for(Cell &c: state.getMap())
	{
		if(c.isPlayer())
			scores[type2ply(c.type)]++;
		if(c.type == color)
			h += 1 + c.staticValue;
	}

	u32 me = 0, enemies = 0;
	u32 bestScore = 0;
	u32 bestPly = 0;
	for(usz i = 0; i < game.defaults.players; i++)
	{
		bool m = i == type2ply(color);

		me += m * scores[i];
		enemies += !m * scores[i];

		if(scores[i] > bestScore)
		{
			bestScore = scores[i];
			bestPly = ply2type(i);
		}
	}

	h -= enemies;

	if(ply2type(bestPly) == color)
		h += 10;

	h += overrides * 5; // * game.aiData.expectedOverriteValue;
	h += bombs * game.aiData.bombValue;

	return h;
}

Quality AI::evalMove(Game &state, Move &move)
{
	stats.durs.eval.start();

#if SAFE_GUARDS > 2
	string save = game.getMap().asString();
#endif

	stats.states++;
	handleSpecials(move);

	Player& futureMe = *state.getPlayers()[type2ply(color)];
	Quality h = 0;

	state.execute(move);

	h += evalState(state);
	h += futureMe.possibleMoves().size();

	if(move.override)
		h /= 2;

	state.undo();


#if SAFE_GUARDS > 2
	if(save != game.getMap().asString())
		throw std::runtime_error("map corruption");
#endif

	stats.durs.eval.stop();

	return h;
}

//###################################
//###################################


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

