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
	return fmt::format("\e[{};{}m", attr, color);
#endif
}
