#include "map.h"
#include <iostream>
#include <fstream>
#include <sstream>

void Win32_Break();
void PrintError(string message, string location);
std::vector<string> splitString(string str, char at);
bool parseMapByFile(string filename, Map* map_out, bool debug_text_out);

int main(int argc, char* argv[])
{
// 	Map map(5,5);
// 	std::cout << map.asString() << std::endl;

	std::string str = "15 0 1 <-> 0 14 5";
	std::string empty_str;
	std::vector<string> poses = splitString(str, *(const char*)" ");

 	for (string& s : poses)
 	{
 		std::cout << s << std::endl;
 	}
 
 	std::cout << std::endl;


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
			std::cout << "--- debug_text_out start ---" << std::endl;
			for (std::string& str : lines)
			{
				std::cout << str << std::endl;
			}			
			std::cout << "--- debug_text_out end ---" << std::endl;
			std::cout << std::endl << std::endl;
		}		

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
		u32 tmp_transitions_from = 0;

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
				u32 temp = str.find(" ") + 1;
				
				tmp_bombs = std::stoi(str.substr(0, temp));
				tmp_bomb_strenght = std::stoi(str.substr(temp, str.length()));
			}
			else if (line_count == 4)
			{
				u32 temp = str.find(" ") + 1;
				
				map_height = std::stoi(str.substr(0, temp));
				map_width = std::stoi(str.substr(temp, str.length()));
				map_out = new Map(map_width, map_height);
							
				//Debug				
 				std::cout << map_out->asString();
 				std::cout << "( Printed within the function parseMapByFile through map_out->asString()  )" << std::endl << std::endl;
			}
			else if (4 < line_count <= 4 + map_height)
			{
				for (char& c : str)
				{						
					//map_out->at(map_x, map_y) = (Cell)c;
					map_x++;
				}

				map_y++;
				map_x = 0;
			}
			else if (line_count > 4 + map_height)
			{
				// Parse Transistion    ex: 15 0 1 <-> 0 14 5
			}


			line_count++;
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