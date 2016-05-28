#pragma once

#include "global.h"

#include <vector>
#include <stack>
#include <istream>

std::vector<string> splitString(const string& str, char at);
std::basic_istream<char>& readline(std::basic_istream<char>& stream, string& str);
string readline(std::basic_istream<char>& stream);

string toLower(string str);


struct Scope
{
	using Callback = std::function<void()>;

	Scope() = default;
	Scope(Callback cb);
	~Scope();

	Scope& operator += (Callback cb);
	Callback operator --();
	Scope operator -- (int);
private:
	std::stack<Callback> cbs;
};
