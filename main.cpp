#include <iostream>
#include "MemoryManager.h"
#include "STLcustomalloc.h"
#include <vector>

int main()
{
	MemoryManager m;
	int* p = (int*)(m.malloc(sizeof(int)));

	*p = 20;
	std::cout << *p << std::endl;
	std::vector<int, STLcustomalloc<int> > v;
	
	for (int i = 0; i < 100; i++)
		v.push_back(i);

	for (int i = 49; i < 100; i++)
		std::cout << v[i] << std::endl;
	
	std::cout << v.back() << std::endl;
	return 0;
}