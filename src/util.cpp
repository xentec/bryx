#include "util.h"
#include "map.h"

#include <cppformat/format.h>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#undef VOID
#endif

std::vector<string> splitString(string str, char at)
{
	std::stringstream stream(str);
	std::string item;
	std::vector<std::string> elems;

	while (std::getline(stream, item, at)) {
		elems.push_back(item);
	}
	return elems;
}

std::basic_istream<char> & readline(std::basic_istream<char> &stream, string &str)
{
	std::basic_istream<char> &ret = std::getline(stream, str);
	if(str.size() && str[str.size()-1] == '\r')
		str = str.substr(0, str.size()-1);

	return ret;
}

std::string readline(std::basic_istream<char> &stream)
{
	string str;
	readline(stream, str);
	return str;
}

void printMapColored(Map * map)
{
	for (usz y = 0; y < map->height; y++)
	{
		for (usz x = 0; x < map->width; x++)
		{
			ConsoleFormat color;
			Cell c = map->at(x, y);
			switch (c)
			{
			case Cell::BONUS:     color = color::GREEN_LIGHT;   break;
			case Cell::CHOICE:    color = color::BLUE_LIGHT;    break;
			case Cell::EMPTY:     color = color::GRAY_LIGHT;    break;
			case Cell::EXPANSION: color = color::CYAN_LIGHT;    break;
			case Cell::INVERSION: color = color::MAGENTA; break;
			case Cell::VOID:      color = color::GRAY;          break;
			default:
				if (Cell::P1 <= c && c <= Cell::P8)
					color = color::YELLOW;
			}

			fmt::print("{}{} ", color, (char)c);
		}
		fmt::print("\n");
	}

	for (const Transistion& t : map->getTransitstions())
		fmt::print("{}\n", t.asString());
}


ConsoleFormat::ConsoleFormat(u8 color, ConsoleFormat::Attribute attr):
	color(color), attr(attr)
{}

std::string ConsoleFormat::asString() const
{
#if _WIN32
	static HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (WORD)color);
	return "";
#else
	return fmt::format("\e[{:d};{:d}m", attr, color);
#endif
}
