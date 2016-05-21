#pragma once

#include "global.h"
#include <fmt/ostream.h>

namespace console
{

struct Format
{
	enum Attribute
	{
		NORMAL = 0, BOLD = 1, UNDERLINE = 4, BLINK = 5, REVERSE = 7, INVISIBLE = 8
	};

#ifdef _WIN32
	enum Color
	{
		BLACK = 0, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE, RESET = WHITE
	};

	inline void setFG(Color c) { fg = c; }
	inline void setBG(Color c) { bg = c; }
#else
	enum Color
	{
		RESET = 0, BLACK = 30, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
	};

	inline void setFG(Color c) { fg = c; }
	inline void setBG(Color c) { bg = c + 10; }
#endif

	Format(u8 fg = RESET, Attribute attr = NORMAL);

	string asString() const;

	Attribute attr;
	u8 fg, bg;


	friend std::ostream& operator<<(std::ostream& os, const Format& format)
	{
		return os << format.asString();
	}
};

namespace color
{
	static const Format RESET;

	static const Format BLACK (Format::BLACK);
	static const Format RED (Format::RED);
	static const Format GREEN (Format::GREEN);
	static const Format BROWN (Format::YELLOW);
	static const Format BLUE (Format::BLUE);
	static const Format MAGENTA (Format::MAGENTA);
	static const Format CYAN (Format::CYAN);
	static const Format GRAY_LIGHT (Format::WHITE);

	static const Format GRAY (Format::BLACK, Format::BOLD);
	static const Format RED_LIGHT (Format::RED, Format::BOLD);
	static const Format GREEN_LIGHT (Format::GREEN, Format::BOLD);
	static const Format YELLOW (Format::YELLOW, Format::BOLD);
	static const Format BLUE_LIGHT (Format::BLUE, Format::BOLD);
	static const Format MAGENTA_LIGHT (Format::MAGENTA, Format::BOLD);
	static const Format CYAN_LIGHT (Format::CYAN, Format::BOLD);
	static const Format WHITE (Format::WHITE, Format::BOLD);
}

}
