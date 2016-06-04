TODO: Rewrite the readme
## C++ Memory Manager	

**How to use it:**
> 1. Include the header
>>#include "~path/MemoryManager.h"
> 2. Create an object of the MemoryManager class using one of the constructors of the class
> **MemoryManager myManager(10*1024)** The constructor of MemoryManager takes 0 or 1 parameters - the size of the data pool in bytes. By default it is 4KB.
> 3. Use the **myManager.malloc(size)** method to allocate size bytes of memory
> **malloc(size)** returns *char* * so make sure to cast it to whatever you want to use
> 4. Use the **myManager.free(ptr)** to free the memory of ptr returned by malloc

**Example:**
```C++
#include <iostream>
#include "MemoryManager.h"

int main()
{
	MemoryManager m(10*1024);
	char* p = m.malloc(150);
	//.....
	//.....
	//.....
	
	m.free(p);
	
	return 0;
}
```