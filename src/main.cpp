#include "map.h"
#include "game.h"
#include "util.h"

#include <iostream>
#include <fstream>
#include <sstream>

void Win32_Break();
void PrintError(string message, string location);
bool parseStartupArguments(int argc, char* argv[], std::string& map_name);
bool parseMapByFile(const string& filename, Game& game, bool debug_text_out);

int main(int argc, char* argv[])
{
#ifdef _WIN32
	system("title bryx");
	system("color 0A");
#endif

	Game game;
	string mapFilePath = "dust.map";

	if (!parseStartupArguments(argc, argv, mapFilePath))
	{
#ifdef __linux__
		PrintError("No map assigned ! Please check your arguments" , "parseStartupArguments");
#endif
	}

	if (!parseMapByFile(mapFilePath, game, true))
	{

		PrintError("Failed to create the game !", "main => parseMapByFile(...) => if(line_count == 4) ..");
		return 0;
	}

	std::cout << "Players: " << game.players << std::endl;
	std::cout << "Overrides: " << game.overrides << std::endl;
	std::cout << "Bombs: " << game.bombs << " (" << game.bombsStrength << ")" << std::endl;
	std::cout << "Map: " << game.map->width << "x" << game.map->height << std::endl;
	std::cout << game.map->asString() << std::endl;


	Win32_Break();
	return 0;
}

void Win32_Break()
{
#ifdef _WIN32
	getchar();
#endif
}

bool parseStartupArguments(int argc, char* argv[], std::string& map_name)
{
	if (argc < 2)
	{
		PrintError("No StartUpArguments given !", "parseStartupArguments");
		return false;
	}

	//std::cout << "argc: " << argc << std::endl;

	for (int i = 1; i < argc - 1; i++)
	{
		std::string arg = argv[i];
		std::string argn = argv[i + 1];

		if (argn.empty())
			break;

		if (arg == "-h")
		{
			//Show Help
			std::cout << "Help ...." << std::endl;
		}
		else if (arg == "-m")
		{
			//Set Map
			std::cout << "map: " << argn << std::endl;
			map_name = argn;
		}
		else if (arg == "-ip")
		{
			//Set IP
			std::cout << "ip: " << argn << std::endl;
		}
		else if (arg == "-port")
		{
			//Set Port
			std::cout << "port: " << argn << std::endl;
		}
	}

	return true;
}

bool parseMapByFile(const string& filename, Game &game, bool debug_text_out)
{
	using std::stoi;

	std::ifstream file(filename);

	if (!file.good())
	{
		PrintError("File not found !", "parseMapByFile => if(file.good()) ..");

		return false;
	}

	game.players = stoi(readline(file));
	game.overrides = stoi(readline(file));

	{
		std::vector<string> s = splitString(readline(file), ' ');
		if(s.size() < 2)
		{
			PrintError("Failed to parse map: bombs delimiter not found", "parseMapByFile");
			return false;
		}
		game.bombs = stoi(s[0]);
		game.bombsStrength = stoi(s[1]);
	}
	{
		std::vector<string> s = splitString(readline(file), ' ');
		if(s.size() < 2)
		{
			PrintError("Failed to parse map: map dimensions delimiter not found", "parseMapByFile");
			return false;
		}
		game.map = new Map(stoi(s[1]), stoi(s[0]));
	}

	for (usz y = 0; y < game.map->height; y++)
	{
		std::string line = readline(file);
		usz lm = (line.length()+1)/2;

		if(lm != game.map->width)
		{
			PrintError(string("Failed to parse map: length of line ")+std::to_string(y+5)
					   +string(" does not match map width: ")
					   +std::to_string(lm)+string(" != ")+std::to_string(game.map->width), "parseMapByFile");
			return false;
		}

		for(usz x = 0; x < line.length(); x++)
		{
			char c = line[x];

			if(c == ' ') continue;
			if(!Map::isCell(c))
			{
				PrintError("Failed to parse map: invalid cell character '" + string(&c) + "' found at "+std::to_string(y+5)+":"+std::to_string(x), "parseMapByFile");
				return false;
			}

			game.map->at(x/2, y) = static_cast<Cell>(c);
		}
	}

	string line;
	while(readline(file, line))
	{
		if(line.empty()) continue;

		// Parse Transistion
		std::vector<string> transistion_parts = splitString(line, ' ');

		Location from({ stoi(transistion_parts.at(0)), stoi(transistion_parts.at(1)) }, static_cast<Direction>(stoi(transistion_parts.at(2)))),
				 to  ({ stoi(transistion_parts.at(4)), stoi(transistion_parts.at(5)) }, static_cast<Direction>(stoi(transistion_parts.at(6))));

#if DEBUG
		std::cout << "####################################" << std::endl;
		std::cout << "Base Line:" << std::endl;
		std::cout << line << std::endl;
		std::cout << "Split Line:" << std::endl;
		std::cout << " 0: " << transistion_parts.at(0)
				  << " 1: " << transistion_parts.at(1)
				  << " 2: " << transistion_parts.at(2)
				  << " 4: " << transistion_parts.at(4)
				  << " 5: " << transistion_parts.at(5)
				  << " 6: " << transistion_parts.at(6)
				  << std::endl;
		std::cout << "####################################" << std::endl;
#endif
		Transistion tr(from, to);

		try {
			game.map->add(tr);
		}
		catch(std::out_of_range& e)
		{
			std::cout << "Failed to set transistion [" << tr.asString() << "]: position is outside the map [" << e.what() << "]" << std::endl;
			return false;
		}
		catch(std::exception& e)
		{
			std::cout << "Failed to set transistion [" << tr.asString() << "]: " << e.what() << std::endl;
			return false;
		}
	}

	//Win32_Break();
	return true;
}

void PrintError(string message, string location)
{
	std::cout	<< "Error: " << std::endl
				<< message << std::endl
				<< "At: " << std::endl
				<< location << std::endl
				<< std::endl;

//	Win32_Break();
}
