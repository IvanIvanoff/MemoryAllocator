#include "MemoryAllocator.h"

#include <iostream>
#include <exception>
#include <new>
#include <assert.h>
#include <algorithm>


struct allocated_block {
	size_t size;
};
struct free_header : public allocated_block {
	free_header* pNext;
	free_header* pPrev;
};

// ALWAYS a power of 2 !!!
const int ALIGNMENT = 8;
//the magic happens with &~(ALIGNMENT -1) => masks out the last power of two minus 1 bits
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT-1))
#define POINTER_ADD(p, n) ((char*)p + n)
#define POINTER_SUB(p, n) ((char*)p - n)

#define FHEADER_SIZE ALIGN(sizeof(free_header))
#define ABLOCK_SIZE ALIGN(sizeof(allocated_block))
const int SPLIT_ABOVE = 8 + 2 * ABLOCK_SIZE;

/*	The constructor of the Memory Allocator
Throws an exception if the memory cannot be allocated
Works with atleast 1mb of memory

@param[in] The size of the heap in megabytes. 10mb by default </param>
*/
MemoryAllocator::MemoryAllocator(int size)
{
	if (size <= 0) throw std::bad_alloc();
	heap_ = new char[size * 1024 * 1024];
	heap_end_ = heap_ + size * 1024 * 1024;

	//The current_free_ must be different from the start of the heap
	//The flists points to the start of the heap and this way we avoid miscaclucations
	current_free_ = reinterpret_cast<free_header*>(heap_) + FHEADER_SIZE;
	current_free_ += ((size_t)current_free_ % ALIGNMENT);
	flist_ = reinterpret_cast<free_header*>(heap_);
	flist_->pNext = flist_;
	flist_->pPrev = flist_;
	flist_->size = 0;
}

/*	The destructor of the Memory Allocator
Frees all the memory
*/
MemoryAllocator::~MemoryAllocator()
{
	//Do not have to delete anything else
	//Only heap_ has been allocated with operator new and thus needs to be freed
	if (heap_)
		delete heap_;
}

/*	Allocates on the heap size bytes of memory and returns pointer to them

@param[in] size The size in bytes which will be allocated
@param[in] throwing Specific enum type that indicates throwing or not throwing an exception
@return a void pointer to a chunk of memory of the given size or nullptr
*/
char* MemoryAllocator::malloc(size_t size, thrw throwing)
{
	assert(size >= 0 && "The size of malloc must be non negative");
	if (size == 0) 	return nullptr;

	size_t requested_size = ALIGN(size + 2 * ABLOCK_SIZE);

	auto ptr = find_fit(requested_size);

	if (ptr == nullptr) {
		if (throwing == DO_THROW) {
			throw std::bad_alloc();
		}
		else {
			return nullptr;
		}
	}
	auto footer = reinterpret_cast<allocated_block*>((char*)ptr + ptr->size - ABLOCK_SIZE);
	ptr->size |= 1;
	footer->size = ptr->size; //size already has its last bit up
	return (char*)POINTER_ADD(ptr, ABLOCK_SIZE);
}

/*	Searches in the explicit free lists for a memory chuck with the requested size.
Also performs splitting if the found memory chunk is too large.

@param[in] size requested size
@return pointer to a free_header from the free list or nullptr if there's none
which satisfies the requirement for the size
*/
free_header* MemoryAllocator::search_in_free_list(size_t size) const
{
	free_header* freeBlock = flist_;
	//Check the free list
	for (freeBlock = freeBlock->pNext;
	freeBlock != flist_;
		freeBlock = freeBlock->pNext
		) {
		if (freeBlock->size >= size) {
			//Split if there is too much memory wasted
			if (freeBlock->size >= size + SPLIT_ABOVE) {
				free_header* new_fheader = (free_header*)((char*)freeBlock + size);
				freeBlock->pNext->pPrev = new_fheader;
				freeBlock->pPrev->pNext = new_fheader;
				new_fheader->pNext = freeBlock->pNext;
				new_fheader->pPrev = freeBlock->pPrev;
				new_fheader->size = freeBlock->size - size;
				freeBlock->size = size;
			}
			else {
				freeBlock->pNext->pPrev = freeBlock->pPrev;
				freeBlock->pPrev->pNext = freeBlock->pNext;
			}
			return freeBlock;
		}
	}
	return nullptr;
}

/*	Implements address fit algorithm to allocate memory

@param[in] size requestested size
@return pointer to the allocated block
*/
allocated_block* MemoryAllocator::find_fit(size_t size)
{
	free_header* freeBlock = search_in_free_list(size);

	if (freeBlock != nullptr) {
		return (allocated_block*)freeBlock;
	}
	//If there's nothing in the free list check after the last allocation
	//This could happen if there is no freed memory
	//If there is not enough memory return nulltpr
	freeBlock = current_free_;

	if ((char*)freeBlock + size < heap_end_) {
		current_free_ = (free_header*)((char*)freeBlock + size);
		freeBlock->size = size;
		return (allocated_block*)(freeBlock);
	}

	return nullptr;
}

/*	Gets a free header and performs coalescence if possible with the
previous and/or next block in the memory

@param[in] ptr pointer to a free header
*/
void MemoryAllocator::coalesce(free_header* ptr)
{
	//Coalescence with next
	free_header* nextBlock = (free_header*)((char*)ptr + ptr->size);
	if ((char*)nextBlock < (char*)current_free_) {
		if ((nextBlock->size & 1) == 0) //check if it's free block
		{
			ptr->size += nextBlock->size;
			ptr->pNext = nextBlock->pNext;
			nextBlock->pNext->pPrev = ptr;
		}
	}

	//Coalescence with previous
	allocated_block* prev = (allocated_block*)((char*)ptr - ABLOCK_SIZE);
	if ((prev->size & 1) == 0) //check if it's a free block
	{
		free_header* prevBlock = (free_header*)((char*)ptr - prev->size);

		prevBlock->size += ptr->size;
		prevBlock->size &= ~1;
		remove_free_header(ptr);
	}
}

/*	Frees the memory which pBlock points to
If invalid address is given the behavious is undefined

@param[in] pBlock pointer to a chunk of memory. pBlock must be returned by malloc
*/
void MemoryAllocator::free(char* pBlock)
{
	if (pBlock == nullptr || pBlock == NULL) return;
	pBlock -= ABLOCK_SIZE;
	free_header* ptr = (free_header*)(pBlock);
	ptr->size &= ~1;
	allocated_block* footer = (allocated_block*)((char*)ptr + ptr->size - ABLOCK_SIZE);
	footer->size &= ~1;

	//Add it to the free list
	add_to_free_list(ptr);

	//coalesence
	coalesce(ptr);
}

/*	Adds new element to the free list

@param[in] ptr pointer to a free header to be added to the list
*/
void MemoryAllocator::add_to_free_list(free_header* ptr) const
{
	char* pBlock = reinterpret_cast<char*>(ptr);
	free_header* spot = flist_->pNext;

	for (spot; spot != flist_; spot = spot->pNext)
	{ //adress fit
		if ((char*)spot < pBlock && (char*)(spot->pNext) > pBlock)
			break;
	}
	ptr->pNext = spot->pNext;
	ptr->pPrev = spot;
	spot->pNext->pPrev = ptr;
	spot->pNext = ptr;
}

/*  Checks if a block of memory is owned by the free list
Used for debugging
@param[in] pBlock pointer to a chunk of memory
@return true if the freelists owns the header, false otherwise
*/
bool MemoryAllocator::flist_owns(const void* pBlock) const
{
	auto p = reinterpret_cast<free_header*>(&pBlock);

	for (auto curr = flist_->pNext;
	curr != flist_;
		curr = curr->pNext)
	{
		if (curr == p)
			return true;
	}

	return false;
}

/* Removes the given header from the explicit free list

@param[in] freeBlock pointer to the header needed to be removed
*/
void MemoryAllocator::remove_free_header(free_header* freeBlock) const
{
	auto spot = flist_;
	for (spot = flist_->pNext;
	spot != flist_;
		spot = spot->pNext)
	{
		if (freeBlock == spot)
		{
			freeBlock->pNext->pPrev = freeBlock->pPrev;
			freeBlock->pPrev->pNext = freeBlock->pNext;
			break;
		}
	}
}

/*	Counts how many free headers are there in the free list
Used only for debugging and testing if coalescence works

@return integer the number of the elements in the free list
*/
int MemoryAllocator::count_in_flist() const
{
	int count = 0;
	for (auto p = flist_->pNext;
	p != flist_;
		p = p->pNext
		) count++;

		return count;
}