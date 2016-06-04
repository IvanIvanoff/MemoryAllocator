TODO: Rewrite the readme
## C++ Memory Manager	

**How to use it:**
* Include the header
```C++
 #include "~path/MemoryManager.h"
```
* Create an object of the MemoryManager class using one of the constructors of the class

> Constructor with one parameter size creates new object with *size bytes of memory
```C++
MemoryManager myManager(size);
```

> The default constructors creates new object with 4KB memory
```C++
MemoryManager myOtherManager();
```

* Use the **myManager.malloc(size)** method to allocate size bytes of memory
> The malloc method returs char* 
```C++
char* ptr = myManager.malloc( sizeof(char) * 20 );
```

> If you want to use type different from char* you should use explicit casts
```C++
int* numArray = (int*)myManager.malloc( sizeof(int) * 100 );
```

> By default **malloc(size)** throws an exception when it fails to allocate memory. The code in the previous examples is equivalent to:
```C++
char* ptr = myManager.malloc( sizeof(char) * 20, DO_THROW );
int* numArray = (int*)myManager.malloc(sizeo(int) * 100, DO_THROW);
```

> If you want to assign nullptr to the pointer when the allocation fails instead of throwing exception use *NO_THROW* as the second argument
```C++
double* dArray = (double*)myManager.malloc( sizeof(double) * 5000, NO_THROW);
```

* Use the **myManager.free(ptr)** to free the memory of ptr returned by malloc

> The free(ptr) method takes a single parameter pointer returned by *malloc(size)* cast back to char* or NULL/nullptr. If other argument is passed the behaviour is undefined.
```C++
myManager.free(ptr);
myManager.free(numArray); // Error
myManager.free((char*)numArray); //OK
char* x;
myManager.free(x); //Undefined behaviour
```


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