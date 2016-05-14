#include "MemoryAllocator.h"

#include <iostream>
#include <exception>
#include <new>
#include <assert.h>
#include <algorithm>
#include <cstdint>

struct allocated_block {
	uint8_t size;
};
struct free_header {
	uint8_t size;
	free_header* pNext;
	free_header* pPrev;
};

// ALWAYS a power of 2 !!!
const int ALIGNMENT = 8;
//the magic happens with &~(ALIGNMENT -1) => masks out the last power of two minus 1 bits
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT-1))
#define PTR_ADD(p, n) ( (char*) (p) + ALIGN(n) )
#define PTR_SUB(p, n) ( (char*) (p) - ALIGN(n) )
#define CAST(elem, type) ( (type)(elem) )
#define FHEADER_SIZE ALIGN(sizeof(free_header))
#define ABLOCK_SIZE ALIGN(sizeof(allocated_block))

//If there is a free block with size >= requested_size + SPLIT_ABOVE
//split the free block into two blocks, first of which has the requested_size
const int SPLIT_ABOVE = 16 + 2 * ABLOCK_SIZE;

/*	
 *	The constructor of the Memory Allocator
 *	Throws an exception if the memory cannot be allocated
 *	If passed less than 4KB , automaticaly allocates 4KB
 *	@param[in] The size of the heap in bytes. 10 * 1024 * 1024 by default.
 */
MemoryAllocator::MemoryAllocator(int size)
{
	if (size <= 4 * 1024) {
		size = 4 * 1024;
	}

	//heap_ starts at address multiple of 32
	heap_ = (char*) (new uint32_t[size / sizeof(uint32_t)]);
	heap_end_ = heap_ + size;

	//The current_free_ must be different from the start of the heap
	//The flists points to the start of the heap and this way we avoid miscaclucations
	current_free_ =  CAST(heap_, free_header*);

	flist_ = CAST(heap_, free_header*);
	PTR_ADD(current_free_, FHEADER_SIZE);
	flist_->pNext = flist_;
	flist_->pPrev = flist_;
	flist_->size = 0;
}

/*	The destructor of the Memory Allocator
 *	Frees all the memory
 */
MemoryAllocator::~MemoryAllocator()
{
	//Do not have to delete anything else
	//Only heap_ has been allocated with operator new and thus needs to be freed
	if (heap_)
		delete heap_;
}

/*	Allocates on the heap size bytes of memory and returns pointer to them
 *	@param[in] size The size in bytes which will be allocated
 *	@param[in] throwing Specific enum type that indicates throwing or not throwing an exception
 *
 *	@return a void pointer to a chunk of memory of the given size or nullptr
*/
char* MemoryAllocator::malloc(size_t size, thrw throwing)
{
	if (size == 0) 	return nullptr;

	size_t requested_size = ALIGN(size + 2 * ABLOCK_SIZE);

	allocated_block* ptr = find_fit(requested_size);

	if (ptr == nullptr) {
		if (throwing == DO_THROW) {
			throw std::bad_alloc();
		}
		else {
			return nullptr;
		}
	}
	allocated_block* footer = reinterpret_cast<allocated_block*>((char*)ptr + ptr->size - ABLOCK_SIZE);
	ptr->size |= 1;
	footer->size = ptr->size; //size already has its last bit up
	
	PTR_ADD(ptr, ABLOCK_SIZE);

	return CAST(ptr, char*);
}

/*	Searches in the explicit free lists for a memory chuck with the requested size.
 *	Also performs splitting if the found memory chunk is too large.

 *	@param[in] size requested size
 *	@return pointer to a free_header from the free list or nullptr if there's none
 *	which satisfies the requirement for the size
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
				//free_header* new_fheader = CAST(PTR_ADD(freeBlock, size), free_header*);
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
 *	@param[in] size requestested size
 *
 *	@return pointer to the allocated block
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
		//current_free_ = CAST(PTR_ADD(freeBlock, size), free_header*);
		freeBlock->size = size;
		return CAST(freeBlock, allocated_block*);
	}

	return nullptr;
}

/*	Gets a free header and performs coalescence if possible with the
 *	previous and/or next block in the memory
 *
 *	@param[in] ptr pointer to a free header
 */
void MemoryAllocator::coalesce(free_header* ptr)
{
	//Coalescence with next
	
	free_header* nextBlock = (free_header*)((char*)ptr + ptr->size);
	//free_header* nextBlock = CAST(PTR_ADD(ptr, ptr->size), free_header*);
	
	if ((char*)nextBlock < (char*)current_free_) {
	//if (CAST(nextBlock, char*) < CAST(current_free_, char*)) {
		if ((nextBlock->size & 1) == 0) //check if it's free block
		{
			ptr->size += nextBlock->size;
			ptr->pNext = nextBlock->pNext;
			nextBlock->pNext->pPrev = ptr;
		}
	}

	//Coalescence with previous
	
	allocated_block* prev = (allocated_block*)((char*)ptr - ABLOCK_SIZE);
	//allocated_block* prev = CAST(PTR_SUB(ptr, ABLOCK_SIZE), allocated_block*);
	if ((prev->size & 1) == 0) //check if it's a free block
	{
		
		free_header* prevBlock = (free_header*)((char*)ptr - prev->size);
		//free_header* prevBlock = CAST(PTR_SUB(ptr,prev->size), free_header*);

		prevBlock->size += ptr->size;
		prevBlock->size &= ~1;
		remove_free_header(ptr);
	}
}

/*	Frees the memory which pBlock points to
 *	If invalid address is given the behavious is undefined
 *
 *	@param[in] pBlock pointer to a chunk of memory. pBlock must be returned by malloc
 */
void MemoryAllocator::free(char* pBlock)
{
	if (pBlock == nullptr || pBlock == NULL) return;
	pBlock = PTR_SUB(pBlock, ABLOCK_SIZE);
	free_header* ptr = CAST(pBlock, free_header*);
	ptr->size &= ~1;
	allocated_block* footer = CAST( ptr, allocated_block*);

	PTR_ADD(footer, ptr->size);
	PTR_SUB(footer, ABLOCK_SIZE);
	footer->size &= ~1;

	//Add it to the free list
	add_to_free_list(ptr);

	//coalesence
	coalesce(ptr);
}

/*	Adds new element to the free list
 *	@param[in] ptr pointer to a free header to be added to the list
 */
void MemoryAllocator::add_to_free_list(free_header* ptr) const
{
	char* pBlock = CAST(ptr, char*);
	free_header* spot = flist_->pNext;

	for (spot; spot != flist_; spot = spot->pNext)
	{ //adress fit
		if (CAST(spot, char*) < pBlock && CAST(spot->pNext, char*) > pBlock)
			break;
	}
	ptr->pNext = spot->pNext;
	ptr->pPrev = spot;
	spot->pNext->pPrev = ptr;
	spot->pNext = ptr;
}

/*  Checks if a block of memory is owned by the free list
 *	Used for debugging
 *	@param[in] pBlock pointer to a chunk of memory
 *
 *	@return true if the freelists owns the header, false otherwise
*/
bool MemoryAllocator::flist_owns(const void* pBlock) const
{
	free_header* p = CAST(pBlock, free_header*);

	for (free_header* curr = flist_->pNext;	curr != flist_;	curr = curr->pNext)	{
		if (curr == p)
			return true;
	}

	return false;
}

/* Removes the given header from the explicit free list
 *	@param[in] freeBlock pointer to the header needed to be removed
 */
void MemoryAllocator::remove_free_header(free_header* freeBlock)
{
	free_header* spot = flist_;
	for (spot = flist_->pNext; spot != flist_; spot = spot->pNext) {
		if (freeBlock == spot){
			freeBlock->pNext->pPrev = freeBlock->pPrev;
			freeBlock->pPrev->pNext = freeBlock->pNext;
			break;
		}
	}
}

/*	Counts how many free headers are there in the free list
 *	Used only for debugging and testing if coalescence works
 *
 *	@return integer the number of the elements in the free list
 */
int MemoryAllocator::count_in_flist() const
{
	int count = 0;
	for (free_header* p = flist_->pNext; p != flist_; p = p->pNext) {
		count++;
	}
	return count;
}