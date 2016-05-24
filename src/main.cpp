#include "map.h"
#include "game.h"
#include "client.h"
#include "util.h"

#include "ai.h"
#include "human.h"

#include <fmt/format.h>

#include <algorithm>
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

	string host = "localhost";
	u16 port = 7777;

	string mapPath;
};

void usage(Options& opts, const string&, usz&, const string*)
{
	fmt::print("usage: {} [options] <spectator|pvp> <map path>\n", opts.progName);
	fmt::print("       {} [options] <client>        <host>\n", opts.progName);
	std::exit(0);
}


void parseArgs(Options& opts, i32 argc, char* argv[])
{

	using Handler = std::function<void(Options&, const string&, usz& i, const string*)>;
	//Handler map = [](Options& opts, const string& arg, const string& param){ return true; };
	static std::unordered_map<string, Handler> handlers =
	{
		{"-h", &usage},
		{"-i", [](Options& opt, const string&, usz& i, const string* arg)
			{
				if(arg)
					opt.mode = Mode::CLIENT, opt.host = *arg, i++;
			}},
		{"-p", [](Options& opt, const string&, usz& i, const string* arg)
			{
				if(arg)
					opt.port = static_cast<u16>(std::stoi(*arg)), i++;
			}},
		{"-nc", [](Options&, const string&, usz& i, const string*){ Map::printColored = false; }},
		{"-na", [](Options&, const string&, usz& i, const string*){ Map::printAnsi = false; }},
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

	usz i = 1;
	for(; i < argc; i++)
	{
		string arg = argv[i];
		auto h = handlers.find(arg);
		if(h == handlers.end())
			break;

		string val, *p = nullptr;
		if(i+1 < argc)
			p = &(val = argv[i+1]);

		h->second(opts, arg, i, p);
	}

	if(opts.mode == Mode::UNKNOWN)
	{
		auto mode = modes.find(argv[i++]);
		if(mode != modes.end())
			opts.mode = mode->second;
		else
			throw std::runtime_error("no mode selected");
	}

	switch(opts.mode)
	{
	case Mode::SPECTATE:
	case Mode::PVP:
		if(i >= argc)
			throw std::runtime_error("no map path specified");
		opts.mapPath = argv[i];
		break;
	case Mode::CLIENT:
		if(i >= argc)
			throw std::runtime_error("no host or IP specified");
		opts.host = argv[i];
		break;
	default:
		throw std::runtime_error("invalid mode: " + string(argv[i-1]));
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
	std::cout.flush();

	if(opts.mode != Mode::CLIENT)
	{
		fmt::print("Loading map {} ... ", opts.mapPath);

		std::ifstream file( opts.mapPath);
		if (!file)
		{
			fmt::print("FAILED!\n");
			fmt::print("File '{}' cannot be read\n",  opts.mapPath);
			return 0;
		}

		Game game;
		game.load(file);

		fmt::print("Done!\n");

		fmt::print("Players: {}\n", game.defaults.players);
		fmt::print("Overrides: {}\n", game.defaults.overrides);
		fmt::print("Bombs: {} ({})\n", game.defaults.bombs, game.defaults.bombsStrength);
		fmt::print("Map: {}x{}\n", game.getMap().width, game.getMap().height);
		game.getMap().print();


		switch(opts.mode)
		{
		case Mode::SPECTATE:
			for(u32 i = 0; i < game.defaults.players; i++)
				game.addPlayer<AI>();
			break;
		case Mode::PVP:
			fmt::print("\n");
			for(u32 i = 0; i < game.defaults.players; i++)
			{
				fmt::print("Player {}: human (h) or computer (c)? ", i+1);
				string input;

				std::cin >> input;
				Player *ply = nullptr;
				if(!input.empty() && toLower(input)[0] == 'h')
				{
					fmt::print("Player {}, enter your name: ", i+1);
					std::cin >> input;
					ply = &game.addPlayer<Human>();
					ply->name = input;
				} else
				{
					ply = &game.addPlayer<AI>();
				}
				fmt::print("Player name set: {}\n", *ply);

			}
			break;
		default:
			return 0;
		}

		game.run();

		std::vector<std::pair<Cell::Type, u32>> scores(game.getPlayers().size(), {Cell::Type::VOID,0});

		for(Cell& c: game.getMap())
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
		game.getMap().print();

		fmt::print("Moves: {}\n", game.stats.moves);
		fmt::print("Scores:\n");
		for(usz i = 0; i < scores.size(); i++)
		{
			fmt::print("{}. Player {}: {}\n", i+1, (char) scores[i].first, scores[i].second);
		}

		fmt::print("Longest move: {} ms\n", game.stats.time.moveMax.count() * 1000);
		fmt::print("Average move: {} ms\n", game.stats.time.moveAvg.count() * 1000);
	}
	else
	{
		Client client;
		try
		{
			client.join(opts.host, opts.port);
		}catch (const std::exception& ex)
		{
			fmt::print(stderr, "Failed to join the game: {}\n", ex.what());
		}
		client.play();
	}
	return 0;
}
