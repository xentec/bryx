#include "map.h"
#include "game.h"
#include "util.h"

#include "ai.h"
#include "human.h"

#include <cppformat/format.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>

#define VERBOSE 1

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

	Mode mode = Mode::UNKNOWN;

	string host;
	string mapPath;
};

void usage(Options& opts, const string& arg, const string& param)
{
	fmt::print("usage: {} [options] <spectator|pvp> <map path>\n", opts.progName);
	fmt::print("       {} [options] <client>        <host>\n", opts.progName);
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

	fmt::print("Players: {}\n", game.defaults.players);
	fmt::print("Overrides: {}\n", game.defaults.overrides);
	fmt::print("Bombs: {} ({})\n", game.defaults.bombs, game.defaults.bombsStrength);
	fmt::print("Map: {}x{}\n", game.map.width, game.map.height);
	game.map.print();

	switch(opts.mode)
	{
	case Mode::SPECTATE:
		for(u32 i = 0; i < game.defaults.players; i++)
			game.addPlayer(new AI(game));
		break;
	case Mode::PVP:
		fmt::print("\n");
		for(u32 i = 0; i < game.defaults.players; i++)
		{
			fmt::print("Player {}: human (h) or computer (c)? ", i+1);
			string input;
			std::cin >> input;
			Player *ply = nullptr;
			if(input.size() && toLower(input)[0] == 'h')
			{
				fmt::print("Player {}, enter your name: ", i+1);
				std::cin >> input;
				ply = &game.addPlayer(new Human(game, input));
			} else
			{
				ply = &game.addPlayer(new AI(game));
			}
			fmt::print("Player {} is {}\n", ply->id, ply->name);

		}
		break;
	case Mode::CLIENT:
		fmt::print("not implemented yet\n");
	default:
		return 0;
	}

	game.run();



	std::vector<std::pair<Cell::Type, u32>> scores(game.players.size(), {Cell::Type::VOID,0});

	for(Cell& c: game.map)
	{
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
	game.map.print();

	fmt::print("Moves: {}\n", game.stats.moves);
	fmt::print("Scores:\n");
	for(usz i = 0; i < scores.size(); i++)
	{
		fmt::print("{}. Player {}: {}\n", i+1, (char) scores[i].first, scores[i].second);
	}

	fmt::print("Longest move: {} ms\n", game.stats.time.moveMax.count() * 1000);
	fmt::print("Average move: {} ms\n", game.stats.time.moveAvg.count() * 1000);

	return 0;
}
