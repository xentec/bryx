#include "console.h"

#include <fmt/format.h>

#ifdef _WIN32
#include <windows.h>
#endif


bool console::quiet = false;


using namespace console;

Format::Format(Color fg, Attribute attr):
	attr(attr), fg(fg), bg(RESET)
{}

string Format::asString() const
{
	fmt::MemoryWriter w;

#ifdef _WIN32
	static HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (WORD)fg);
#else
	w << "\x1b[";
	w << attr;
	if(fg) { w << ";" << fg; }
	if(bg) { w << ";" << bg+10; }
	w << "m";
#endif
	return w.str();

}
