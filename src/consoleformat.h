#pragma once

#include "global.h"

#include <ostream>

struct ConsoleFormat
{
	enum Attribute
	{
		NORMAL = 0, BOLD = 1, UNDERLINE = 4, BLINK = 5, REVERSE = 7, INVISIBLE = 8
	};

#ifdef _WIN32
	enum Color
	{
		BLACK = 0, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE
	};

	inline void setFG(Color c) { fg = c; }
	inline void setBG(Color c) { bg = c; }
#else
	enum Color
	{
		RESET = 0, BLACK = 30, RED = 31, GREEN = 32, YELLOW = 32, BLUE = 34, MAGENTA = 35, CYAN = 36, WHITE = 37
	};

	inline void setFG(Color c) { fg = c; }
	inline void setBG(Color c) { bg = c + 10; }
#endif

	ConsoleFormat(u8 fg = 0, Attribute attr = NORMAL);

	string asString() const;

	Attribute attr;
	u8 fg, bg;


	friend std::ostream& operator<<(std::ostream& os, const ConsoleFormat& format)
	{
		return os << format.asString();
	}
};

namespace color
{
	static const ConsoleFormat RESET;

	static const ConsoleFormat BLACK (ConsoleFormat::BLACK);
	static const ConsoleFormat RED (ConsoleFormat::RED);
	static const ConsoleFormat GREEN (ConsoleFormat::GREEN);
	static const ConsoleFormat BROWN (ConsoleFormat::YELLOW);
	static const ConsoleFormat BLUE (ConsoleFormat::BLUE);
	static const ConsoleFormat MAGENTA (ConsoleFormat::MAGENTA);
	static const ConsoleFormat CYAN (ConsoleFormat::CYAN);
	static const ConsoleFormat GRAY_LIGHT (ConsoleFormat::WHITE);

	static const ConsoleFormat GRAY (ConsoleFormat::BLACK, ConsoleFormat::BOLD);
	static const ConsoleFormat RED_LIGHT (ConsoleFormat::RED, ConsoleFormat::BOLD);
	static const ConsoleFormat GREEN_LIGHT (ConsoleFormat::GREEN, ConsoleFormat::BOLD);
	static const ConsoleFormat YELLOW (ConsoleFormat::YELLOW, ConsoleFormat::BOLD);
	static const ConsoleFormat BLUE_LIGHT (ConsoleFormat::BLUE, ConsoleFormat::BOLD);
	static const ConsoleFormat MAGENTA_LIGHT (ConsoleFormat::MAGENTA, ConsoleFormat::BOLD);
	static const ConsoleFormat CYAN_LIGHT (ConsoleFormat::CYAN, ConsoleFormat::BOLD);
	static const ConsoleFormat WHITE (ConsoleFormat::WHITE, ConsoleFormat::BOLD);
}
