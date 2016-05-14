#pragma once
#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__

/*! \file MemoryAllocator.h
\brief Header file for the memory allocator.

@author Ivan Aleksandrov Ivanov
@version 1.0

*/
#include <cstdint>
struct allocated_block;
struct free_header;

enum thrw {
	DO_THROW,
	NO_THROW
};


/*! Memory allocator is a custom and simple memory allocator. Given a size in bytes it 
    allocates, frees and maintains the memory.
 */
class MemoryAllocator
{
public:
	MemoryAllocator(int64_t = 4 * 1024);				//!< Creates new allocator with the given size in megabytes
	~MemoryAllocator();									//!< Destroys and frees the memory
	MemoryAllocator(const MemoryAllocator&) = delete;	//!< Cannot use copy constructor
	MemoryAllocator(MemoryAllocator&&) = delete;		//!< Cannot use move constructor
	void operator=(const MemoryAllocator&) = delete;	//!< Cannot use copy asignment operator
	void operator=(MemoryAllocator&&) = delete;			//!< Cannot use move assignment operator
public:
	char* malloc(size_t, thrw = DO_THROW);				//!< Allocates size bytes and returs pointer to them
	allocated_block* find_fit(size_t);					//!< Find where to allocate the 
	void free(char*);									//!< Frees the memory pBlocks points to		
	void remove_free_header(free_header*);			//!< Removes a free header from the free list
	int count_in_flist() const;							//!< Counts the headers in the free list
private:
	bool flist_owns(const void*) const;					//!< Checks if pBlocks is pointer owned by the MA
	void add_to_free_list(free_header*) const;			//!< Adds elem to the free list
	void coalesce(free_header*);						//!< Coalesce free header with its heighbours
	free_header* search_in_free_list(size_t size)const;	//!< Searches for a free header with the given size
private:
	char* heap_;										//!< Pointer to the memory pool of the MA
	char* heap_end_;									//!< Pointer to the end of the memory pool
	free_header* flist_;								//!< Vector of lists of headers									
	free_header* current_free_;							//!< Pointer to the free not allocated header in the heap
};

#endif //__MEMORY_ALLOCATOR_H__