#pragma once

#include "global.h"

#include <vector>
#include <istream>

std::vector<string> splitString(string str, char at);
std::basic_istream<char>& readline(std::basic_istream<char>& stream, string& str);
std::string readline(std::basic_istream<char>& stream);

string toLower(string str);

