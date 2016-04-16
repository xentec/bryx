#include "map.h"
#include "game.h"
#include "util.h"

#include <cppformat/format.h>

#include <fstream>
#include <functional>


enum class Mode
{
	SPECTATE, PVP, CLIENT, UNKNOWN = 0xFF
};

string mode2str(Mode mode)
{
	switch(mode)
	{
	case Mode::SPECTATE: return "A.I. Battle";
	case Mode::PVP:      return "Player vs. Player";
	case Mode::CLIENT:   return "A.I. Tournament";
	default:
		return "unknown, report me";
	}
}

struct Options
{
	string progName;

	Mode mode;

	string host;
	string mapPath;
};


template<class T>
inline
T input(Cell::Type player, string key)
{
	T var;
	fmt::print("P{} {} > ", (char) player, key);
//	std::cin >> var;
	return var;
}

void usage(Options& opts, const string& arg, const string& param)
{
	fmt::print("usage: {} [options] <spectator|pvp> <map path>\n", opts.progName);
	fmt::print("       {} [options] <client> <host>\n", opts.progName);
	std::exit(0);
}


void parseArgs(Options& opts, i32 argc, char* argv[])
{

	using Handler = std::function<void(Options&, const string&, const string&)>;
	//Handler map = [](Options& opts, const string& arg, const string& param){ return true; };
	static std::unordered_map<string, Handler> handlers =
	{
		{"-h", &usage},
	};

	static std::unordered_map<string, Mode> modes =
	{
		{ "spec", Mode::SPECTATE },
		{ "spectate", Mode::SPECTATE },
		{ "pvp", Mode::PVP },
		{ "client", Mode::CLIENT }
	};

	opts.progName = argv[0];
	{
		usz pos = opts.progName.rfind('/');
		if(pos != string::npos)
			opts.progName = opts.progName.substr(pos+1);
	}

	for(i32 i = 1; i < argc; i++)
	{
		string arg = argv[i];
		auto h = handlers.find(arg);
		if(h != handlers.end())
			h->second(opts, arg, argv[i]);
	}


	opts.mode = Mode::UNKNOWN;
	if(argc < 2)
		throw std::runtime_error("no mode selected");

	auto mode = modes.find(argv[1]);
	if(mode != modes.end())
		opts.mode = mode->second;

	switch(opts.mode)
	{
	case Mode::SPECTATE:
	case Mode::PVP:
		if(argc < 3)
			throw std::runtime_error("no map path specified");
		opts.mapPath = argv[2];
		break;
	case Mode::CLIENT:
		if(argc < 3)
			throw std::runtime_error("no host or IP specified");
		break;
	default:
		throw std::runtime_error("invalid mode: " + string(argv[1]));
	}
}


int main(int argc, char* argv[])
{
	Options opts;

	try {
		parseArgs(opts, argc, argv);
	} catch(std::exception& ex)
	{
		fmt::print("Failed to parse arguments: {}\n", ex.what());
		return 0;
	}

	fmt::print("Bryx - a ReversiXT implementation\n");
	fmt::print("Mode: {}\n", mode2str(opts.mode));
	fmt::print("Loading map {} ... ", opts.mapPath);

	std::ifstream file( opts.mapPath);
	if (!file)
	{
		fmt::print("FAILED!\n");
		fmt::print("File '{}' cannot be read\n",  opts.mapPath);
		return 0;
	}

	Game game = Game::load(file);

	fmt::print("Done!\n");

	fmt::print("Players: {}\n", game.players.size());
	fmt::print("Overrides: {}\n", game.overrides);
	fmt::print("Bombs: {} ({})\n", game.bombs, game.bombsStrength);
	fmt::print("Map: {}x{}\n", game.map->width, game.map->height);
	game.map->print();

	u32 moveNum = 0;
	u32 player = 0;
	bool end = true;

	switch(opts.mode)
	{
	case Mode::SPECTATE:
	{
		srand(time(NULL));
		do
		{
			fmt::print("\n");
			fmt::print("Player {}\n", (char) game.me);
			fmt::print("##########\n");

			std::vector<Move> moves = game.possibleMoves();

			u32 stoneSum = 0;
			u32 stoneMax = 0;
			std::vector<Move*> bestMoves;
			for(Move& m: moves)
			{
				int stones = m.stones.size();
				stoneSum += stones;
				if(stones > stoneMax)
				{
					bestMoves.clear();
					stoneMax = stones;
					bestMoves.push_back(&m);
					continue;
				}
				if(stones == stoneMax)
					bestMoves.push_back(&m);
			}

			if(moves.size())
			{
/*				fmt::print("{} moves with {} possible captures\n", (char)game.me, moves.size(), stoneMax);

				for(Move& move: moves)
				{
					fmt::print("{}>{} --> {}\n\n", move.start.pos, dir2str(move.dir), move.end->pos);
					std::unordered_set<Vec2> hl;
					hl.insert(move.start.pos);
					hl.insert(move.end->pos);

					for(Cell* c: move.stones)
						hl.insert(c->pos);

//					game.map->print(hl);
				}

//				fmt::print("Best moves being:\n");
//				for(Move* bm: bestMoves)
//					fmt::print("\t{}>{} --> {}\n", bm->start.pos, dir2str(bm->dir), bm->end->pos);
*/
//				Move& move = *bestMoves[bestMoves.size() > 1 ? rand() % (bestMoves.size()-1) : 0];
				Move& move = moves[moves.size() > 1 ? rand() % (moves.size()-1) : 0];
				std::unordered_set<Vec2> hl;
				hl.insert(move.start.pos);
				hl.insert(move.end->pos);

				for(Cell* c: move.stones)
					hl.insert(c->pos);

				game.execute(move);
				game.map->print(hl);
				moveNum++;
			} else
			{
				fmt::print("Cannot move\n", (char) game.me);
			}
	/*
			Vec2 from;
			string dirStr;

			from = input<Vec2>(game.me, "Start [Vec2]"); fmt::print("\n");
			from = { 8, 2 };

			game.map->print({from});

			dirStr = input<string>(game.me, "Direction [Dir]");	fmt::print("\n");
			dirStr = "W";


			Direction dir = str2dir(dirStr);
			Move move { game.map->at(from), dir };

			Move::Error res = game.testMove(move);
			fmt::print("Errors? {}\n", Move::err2str(res));

			if(res != Move::Error::NONE) return 0;
					u32 ply = (u32) c.type - ;
			for(Cell* c: move.stones)
				c->type = game.me;
	*/
			player = (player+1) % game.players.size();
			game.me = (Cell::Type)((u32)Cell::Type::P1 + player);
			end = end && moves.empty();
			if(player == 0)
			{
				if(end)
					break;
				end = true;
			}
		} while(true);
	}
	break;
	case Mode::PVP:
		fmt::print("not implemented yet\n"); return 0;
		break;
	case Mode::CLIENT:
		fmt::print("not implemented yet\n"); return 0;
		break;
	default: return 0;
	}


	std::vector<std::pair<Cell::Type, u32>> scores(game.players.size(), {Cell::Type::VOID,0});

	for(i32 x = 0; x < game.map->width; x++)
	for(i32 y = 0; y < game.map->height; y++)
	{
		Cell &c = game.map->at(x, y);
		if(c.isPlayer())
		{
			auto& e = scores[(usz)c.type - (usz)Cell::Type::P1];
			e.first = c.type;
			e.second++;
		}
	}
	std::sort(scores.begin(), scores.end(), [=](std::pair<Cell::Type, u32>& a, std::pair<Cell::Type, u32>& b)
	{
		return a.second > b.second;
	});

	fmt::print("\n########\n");
	fmt::print("GAME SET\n");
	fmt::print("########\n\n");
	game.map->print();

	fmt::print("Moves: {}\n", moveNum);
	fmt::print("Scores:\n");
	for(usz i = 0; i < scores.size(); i++)
	{
		fmt::print("{}. Player {}: {}\n", i+1, (char) scores[i].first, scores[i].second);
	}

	return 0;
}
