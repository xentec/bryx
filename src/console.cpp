#include "console.h"

#include <fmt/format.h>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace console;

Format::Format(Color fg, Format::Attribute attr):
	attr(attr), fg(fg), bg(RESET)
{}

std::string Format::asString() const
{
#ifdef _WIN32
	static HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (WORD)fg);
	return "";
#else
	fmt::MemoryWriter w;
	w << "\x1b[";
	w << attr;
	if(fg) { w << ";" << fg; }
	if(bg) { w << ";" << bg+10; }
	w << "m";
	return w.str();
#endif
}
