#pragma once

#include "global.h"

#include <vector>
#include <stack>
#include <istream>

std::vector<string> splitString(const string& str, char at);
std::basic_istream<char>& readline(std::basic_istream<char>& stream, string& str);
string readline(std::basic_istream<char>& stream);

string toLower(string str);

// Thanks to http://stackoverflow.com/a/4609795
template <typename T>
isz sgn(T val) { return (T(0) < val) - (val < T(0)); }

// Don't even ask me, why this should be here
template<class iter>
struct IterWrap
{
	IterWrap():
		itr()
	{}

	IterWrap(iter itr):
		itr(itr)
	{}

	IterWrap<iter>& operator +=(isz steps)
	{
		i32 dir = sgn(steps);

		if(dir < 0)
			while(steps++)
				itr--;
		else
			while(steps--)
				itr++;

		return *this;
	}


	IterWrap<iter>& operator=(const iter& other)
	{
		itr = other;
		return *this;
	}

	IterWrap<iter>& operator=(const IterWrap<iter>& other)
	{
		itr = other.itr;
		return *this;
	}

	bool operator==(const iter& other) const
	{
		return itr == other;
	}
	bool operator!=(const iter& other) const
	{
		return itr != other;
	}
	bool operator!=(const IterWrap<iter>& other) const
	{
		return itr != other.itr;
	}
	typename iter::reference operator*() { return *itr; }
	typename iter::pointer operator->()  { return &*itr;  }
protected:
	iter itr;
};
