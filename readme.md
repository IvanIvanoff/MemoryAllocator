TODO: Rewrite the readme
## C++ Memory Manager	

**How to use it:**
* Include the header
```C++
 #include "~path/MemoryManager.h"
```
* Create an object of the MemoryManager class using one of the constructors of the class
```C++
MemoryManager myManager(size);
```
Constructor with one parameter size creates new object with *size bytes of memory
```C++
MemoryManager myOtherManager();
```
The default constructors creates new object with 4KB memory
* Use the **myManager.malloc(size)** method to allocate size bytes of memory
> **malloc(size)** returns *char* * so make sure to cast it to whatever you want to use
* Use the **myManager.free(ptr)** to free the memory of ptr returned by malloc

**Example:**
```C++
#include <iostream>
#include "MemoryManager.h"

int main()
{
	MemoryManager m(10*1024);
	char* p = m.malloc(150);
	
	int* a = (int*)m.malloc(sizeof(int) * 10);
	a[5] = 10;
	std::cout<<a[5]<<std::endl; //Expect to print 10
	//.....
	//.....
	//.....
	m.free((char*)a);
	m.free(p);
	
	return 0;
}
```