#include "map.h"
#include "game.h"

#include <iostream>
#include <fstream>
#include <sstream>

void Win32_Break();
void PrintError(string message, string location);
std::vector<string> splitString(string str, char at);
bool parseMapByFile(const string& filename, Game& game, bool debug_text_out);

int main(int argc, char* argv[])
{
	Game game;

	string mapFilePath = "Read.txt";
	if(argc >= 2) // TODO: better arg parsing with getopt
	{
		mapFilePath.assign(argv[1]);
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

std::vector<string> splitString(string str, char at)
{
	std::vector<u32> positions;
	std::vector<string> retMe;
	int count = 0;

	if (str.empty())
	{
		PrintError("Split string failed during empty input !", "splitString => if (str.empty()) ..");
		return retMe;
	}

	positions.push_back(0);
	for (char& c : str)
	{
		if (c == at)
		{
			positions.push_back(count);
		}

		count++;
	}

	for (int i = 0; i < positions.size() - 1; i++)
	{
		if (positions.at(i) == 0)
			retMe.push_back(str.substr(positions.at(i), positions.at(i + 1)));
		else
			retMe.push_back(str.substr(positions.at(i) + 1, positions.at(i + 1) - positions.at(i) - 1));
	}

	retMe.push_back(str.substr(positions.at(positions.size() - 1) + 1, str.length() - positions.size() - 1));

	return retMe;
}

bool parseMapByFile(const string& filename, Game &game, bool debug_text_out)
{
	std::ifstream file(filename);

	if (!file.good())
	{
		PrintError("File not found !", "parseMapByFile => if(file.good()) ..");
		return false;
	}

	bool valid = false;
	string line;
	for (usz line_count = 1; std::getline(file, line); line_count++)
	{
		if (line_count == 1)
		{
			game.players = std::stoi(line);
		}
		else if (line_count == 2)
		{
			game.overrides = std::stoi(line);
		}
		else if (line_count == 3)
		{
			string::size_type temp = line.find(" ");
			if(temp == string::npos)
			{
				PrintError("Failed to parse map: bombs delimiter not found", "parseMapByFile");
				return false;
			}

			game.bombs = std::stoi(line.substr(0, temp));
			game.bombsStrength = std::stoi(line.substr(temp+1, line.length()));
		}
		else if (line_count == 4)
		{
			string::size_type temp = line.find(" ");
			if(temp == string::npos)
			{
				PrintError("Failed to parse map: map dimensions delimiter not found", "parseMapByFile");
				return false;
			}

			game.map = new Map(std::stoi(line.substr(temp, line.length())), std::stoi(line.substr(0, temp)));
		}
		else if (4 < line_count && line_count <= 4 + game.map->height)
		{
			for(string::size_type i = 0; i < line.length(); i++)
			{
				char c = line[i];

				if(c == ' ') continue;
				if(!Map::isCell(c))
				{
					PrintError("Failed to parse map: invalid cell character found", "parseMapByFile");
					return false;
				}

				game.map->at(i/2, line_count-5) = static_cast<Cell>(c);
			}
		}
		else if (line_count > 4 + game.map->height)
		{
			valid = true;
			// Parse Transistion
		}
	}
	std::cout << game.map->height << std::endl;

	if(!valid)
	{
		PrintError("Failed to parse map: file not complete !", "parseMapByFile => if(!valid)");
		return false;
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

	Win32_Break();
}
