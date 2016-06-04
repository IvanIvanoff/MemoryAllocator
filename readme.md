TODO: Rewrite the readme
## C++ Memory Manager	

**How to use it:**
> 1. Include the header
>>#include "~path/MemoryManager.h"
> 2. Create an object of the MemoryManager class
>> MemoryManager myManager(10*1024*1024);
>> The constructor of MemoryManager takes 1 parameter - the size of the data pool in bytes. By default it is 4KB.
> 3. Use the *myManager.malloc(size)* method to allocate size bytes of memory.
> 4. Use the *myManager.free(ptr)* to free the memory of ptr returned by malloc

Example:
