#include "util.h"

#include <cppformat/format.h>
#include <sstream>

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
	return "";
#else
	return fmt::format("\e[{:d};{:d}m", attr, color);
#endif
}
