#pragma once

#include "global.h"
#include <fmt/ostream.h>

namespace console
{

extern bool quiet;
extern bool colorfull;

struct Format
{
	enum Attribute
	{
		NORMAL = 0, BOLD = 1, UNDERLINE = 4, BLINK = 5, INVERSE = 7, INVISIBLE = 8
	};

	enum Color
	{
#ifdef _WIN32
		BLACK = 0, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE, RESET = WHITE
#else
		RESET = 0, BLACK = 30, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
#endif
	};

	inline void setFG(Color c) { fg = c; }
	inline void setBG(Color c) { bg = c; }

	Format(Color fg = RESET, Attribute attr = NORMAL);

	string asString() const;

	Attribute attr;
	Color fg, bg;


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


template <typename... Args>
void print(const char *format, const Args & ... args)
{
	if(!console::quiet)
		fmt::print(format, args...);
}

template <typename... Args>
inline
void println(const char *format, const Args & ... args)
{
	print(format, args...);
	print("\n");
}

inline
void println()
{
	print("\n");
}


template <typename... Args>
inline
void println(console::Format f, const char *format, const Args & ... args)
{
	print("{}", f);
	print(format, args...);
	print("{}\n", console::color::RESET);
}
