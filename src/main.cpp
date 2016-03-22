
#include "map.h"
#include <iostream>


int main(int argc, char* argv[])
{
	Map map(5,5);
	std::cout << map.asString() << std::endl;

#ifdef _WIN32
	getchar();
#endif
	return 0;
}
