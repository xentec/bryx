#include "util.h"

std::vector<string> splitString(string str, char at)
{
	std::vector<u32> positions;
	std::vector<string> retMe;
	int count = 0;

	if (str.empty())
	{
		PrintError("Split string failed during empty input !", "splitString => if (str.empty()) ..");
		return retMe;
	}

	positions.push_back(0);
	for (char& c : str)
	{
		if (c == at)
		{
			positions.push_back(count);
		}

		count++;
	}

	for (int i = 0; i < positions.size() - 1; i++)
	{
		if (positions.at(i) == 0)
			retMe.push_back(str.substr(positions.at(i), positions.at(i + 1)));
		else
			retMe.push_back(str.substr(positions.at(i) + 1, positions.at(i + 1) - positions.at(i) - 1));
	}

	retMe.push_back(str.substr(positions.at(positions.size() - 1) + 1, str.length() - positions.size() - 1));

	return retMe;
}

std::basic_istream<char> & readline(std::basic_istream<char> &stream, string &str)
{
	std::basic_istream<char> &ret = std::getline(stream, str);
	if(str[str.size()-1] == '\r')
		str = str.substr(0, str.size()-1);

	return ret;
}
