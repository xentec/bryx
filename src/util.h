#pragma once

#include "global.h"

#include <vector>
#include <istream>

std::vector<string> splitString(string str, char at);
std::basic_istream<char>& readline(std::basic_istream<char>& stream, string& str);
std::string readline(std::basic_istream<char>& stream);

struct ConsoleFormat
{
	enum Attribute
	{
		NORMAL = 0, BOLD = 1, UNDERLINE = 4, BLINK, REVERSE = 7, INVISIBLE
	};

	ConsoleFormat(u8 color = 0, Attribute attr = NORMAL);

	string asString() const;

	friend std::ostream& operator<<(std::ostream& os, const ConsoleFormat& format)
	{
		return os << format.asString();
	}

	Attribute attr;
	u8 color;
};



namespace color
{
	static const ConsoleFormat RESET(0);

	static const ConsoleFormat BLACK(30);
	static const ConsoleFormat RED(31);
	static const ConsoleFormat GREEN(32);
	static const ConsoleFormat BLUE(34);
	static const ConsoleFormat MAGENTA(35);
	static const ConsoleFormat CYAN(36);
	static const ConsoleFormat BROWN(36);
	static const ConsoleFormat GRAY_LIGHT(37);

	static const ConsoleFormat GRAY(30, ConsoleFormat::BOLD);
	static const ConsoleFormat RED_LIGHT(31, ConsoleFormat::BOLD);
	static const ConsoleFormat GREEN_LIGHT(32, ConsoleFormat::BOLD);
	static const ConsoleFormat YELLOW(33, ConsoleFormat::BOLD);
	static const ConsoleFormat BLUE_LIGHT(34, ConsoleFormat::BOLD);
	static const ConsoleFormat MAGENTA_LIGHT(35, ConsoleFormat::BOLD);
	static const ConsoleFormat CYAN_LIGHT(36, ConsoleFormat::BOLD);
	static const ConsoleFormat WHITE(37, ConsoleFormat::BOLD);
}
