#include "consoleformat.h"

#include <cppformat/format.h>

ConsoleFormat::ConsoleFormat(u8 fg, ConsoleFormat::Attribute attr):
	attr(attr), fg(fg), bg(0)
{}

std::string ConsoleFormat::asString() const
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
