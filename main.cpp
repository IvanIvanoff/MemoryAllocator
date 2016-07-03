#include <iostream>
#include "MemoryManager.h"
#include "STLcustomalloc.h"
#include <vector>

int main()
{
	
	std::vector<int, STLcustomalloc<int> > v;
	
	for (int i = 0; i < 1000; i++)
		v.push_back(i);

	for (int i = 0; i < 1000; i++)
		std::cout << v[i] << std::endl;
	
	std::cout << v.back() << std::endl;
	return 0;
}