#include "console.h"

#include <fmt/format.h>

using namespace console;

Format::Format(u8 fg, Format::Attribute attr):
	attr(attr), fg(fg), bg(0)
{}

std::string Format::asString() const
{
#if _WIN32
	static HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (WORD)fg);
	return "";
#else
	fmt::MemoryWriter w;
	w.write("\e[{}", attr);

	if(fg) w.write(";{}", fg);
	if(bg) w.write(";{}", bg);

	w.write("m");
	return w.str();
#endif
}
