#pragma once

#include "global.h"

#include <vector>
#include <stack>
#include <istream>

std::vector<string> splitString(const string& str, char at);
std::basic_istream<char>& readline(std::basic_istream<char>& stream, string& str);
string readline(std::basic_istream<char>& stream);

string toLower(string str);

template<class T, usz SIZE = 5>
struct LowPass
{
	usz i = 0;
	std::array<T, SIZE> data;

	void push(T&& val)
	{
		data[i] = val;
		i = (i+1) % data.size();
	}

	T average() const
	{
		T sum{};

		for(const T& v: data)
			sum += v;

		return sum / data.size();
	}
};

template<class T>
struct RunningAverage
{
	std::deque<T> data;

	T average() const
	{
		T sum{};

		for(const T& v: data)
			sum += v;

		return sum / data.size();
	}
};
