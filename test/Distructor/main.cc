

#include "ExClass.h"
#include <cstdio>
#include <map>
#include <string>
#include <algorithm>

void DeleteExClass(std::pair<std::string, ExClass*> p) {
	delete p.second;
	p.second = 0;
}

int main(int argc, char **argv) {
	std::map<std::string, ExClass*> themap;
	themap["a"] = new ExClass();
	themap["b"] = new ExClass();
	themap["c"] = new ExClass();
	themap["d"] = new ExClass();
	themap["e"] = new ExClass();

	for_each(themap.begin(), themap.end(), DeleteExClass);
	themap.clear();
	printf("Size %u\n", themap.size());
}

