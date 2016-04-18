#pragma once

#include "global.h"

#include <vector>
#include <istream>

std::vector<string> splitString(string str, char at);
std::basic_istream<char>& readline(std::basic_istream<char>& stream, string& str);
std::string readline(std::basic_istream<char>& stream);

string toLower(string str);

struct ConsoleFormat
{
	enum Attribute
	{
		NORMAL = 0, BOLD = 1, UNDERLINE = 4, BLINK = 5, REVERSE = 7, INVISIBLE = 8
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
#if _WIN32
	static const ConsoleFormat RESET(7);

	static const ConsoleFormat BLACK(0);
	static const ConsoleFormat RED(4);
	static const ConsoleFormat GREEN(2);
	static const ConsoleFormat BLUE(1);
	static const ConsoleFormat MAGENTA(5);
	static const ConsoleFormat CYAN(3);
	static const ConsoleFormat BROWN(6);
	static const ConsoleFormat GRAY_LIGHT(7);

	static const ConsoleFormat GRAY(8, ConsoleFormat::BOLD);
	static const ConsoleFormat RED_LIGHT(12, ConsoleFormat::BOLD);
	static const ConsoleFormat GREEN_LIGHT(10, ConsoleFormat::BOLD);
	static const ConsoleFormat YELLOW(14, ConsoleFormat::BOLD);
	static const ConsoleFormat BLUE_LIGHT(9, ConsoleFormat::BOLD);
	static const ConsoleFormat MAGENTA_LIGHT(13, ConsoleFormat::BOLD);
	static const ConsoleFormat CYAN_LIGHT(11, ConsoleFormat::BOLD);
	static const ConsoleFormat WHITE(15, ConsoleFormat::BOLD);
#else
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
#endif
}
