#include "map.h"
#include <iostream>
#include <fstream>
#include <sstream>

void Win32_Break();
void PrintError(string message, string location);
bool parseMapByFile(string filename, Map* map_out, bool debug_text_out);

int main(int argc, char* argv[])
{
// 	Map map(5,5);
// 	std::cout << map.asString() << std::endl;


	Map* debug_map = nullptr;
	if (parseMapByFile("Read.txt", debug_map, true))
	{
		if (debug_map != nullptr)
		{
			std::cout << debug_map->asString() << std::endl;
		}
		else
		{
			PrintError("Failed to create the map !", "main => if(debug_map != nullptr) .. || parseMapByFile(...) => if(line_count == 4) ..");
		}
	}
	else
	{
		//No output needed, information comes from function itself
		//PrintError("Failed to parse the map !", "main => parseMapByFile(...) call");
	}

	Win32_Break();
	return 0;
}

void Win32_Break()
{
#ifdef _WIN32
	getchar();
#endif
}

bool parseMapByFile(string filename, Map* map_out, bool debug_text_out)
{
	std::ifstream file(filename);
	std::vector<std::string> lines;

	if (file.good())
	{
		std::string str;
		while (std::getline(file, str))
		{
			lines.push_back(str);
		}
	}
	else
	{
		PrintError("File not found !", "parseMapByFile => if(file.good()) ..");
		return false;
	}

	file.close();

	if (!lines.empty())
	{
		if (debug_text_out)
		{
			for (std::string& str : lines)
			{
				std::cout << str << std::endl;
			}
			
			//Win32_Break();
			return true;
		}
		else
		{
			u32 line_count = 1;
			u32 map_width = 0;
			u32 map_height = 0;
			u32 map_x = 0;
			u32 map_y = 0;

			//Temporary Declarations
			u32 tmp_players = 0;
			u32 tmp_overwritestones = 0;
			u32 tmp_bombs = 0;
			u32 tmp_bomb_strenght = 0;

			u32 temp_transitions_from = 0;

			for (std::string& str : lines)
			{
				if (line_count == 1)
				{
					tmp_players = atoi(str.c_str());
				}
				else if (line_count == 2)
				{
					tmp_overwritestones = atoi(str.c_str());
				}
				else if (line_count == 3)
				{					
					// Split Bomb String and fill into tmp variables					
				}
				else if (line_count == 4)
				{
					// Split Size String and fill into tmp variables
					map_out = new Map(map_width, map_height);
				}
				else if (4 < line_count <= 4 + map_height)
				{
					for (char& c : str)
					{
						//map_out[map_x][map_y] = c;										
						map_x++;
					}

					map_y++;
					map_x = 0;
				}
				else if (line_count > 4 + map_height)
				{
					// Parse Transistion
				}
			}

			//Win32_Break();
			return true;
		}
		
		return true;
	}
	else
	{
		PrintError("vector 'lines' empty !", "parseMapByFile => if(lines.empty()) ..");		
		return false;
	}
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