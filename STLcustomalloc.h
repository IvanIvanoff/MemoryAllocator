#pragma once
#ifndef __STL_CUSTOM_ALLOCATOR_H__
#define __STL_CUSTOM_ALLOCATOR_H__
#include <cstddef>
#include "MemoryManager.h"

#define KB_64 (64*1024)

#undef SHOW_ALLOC_MSG


template<typename T>
class STLcustomalloc
{
public:
	//typedefs
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	//ctors & dtor
public:
	STLcustomalloc();
	STLcustomalloc(std::size_t n);
	~STLcustomalloc() {};
	template <class U> STLcustomalloc(const STLcustomalloc<U>& other);
public:
	T* allocate(std::size_t n);
	void deallocate(T* p, std::size_t n);
	MemoryManager* m;
private:
};

template <class T, class U>
bool operator==(const STLcustomalloc<T>&, const STLcustomalloc<U>&);
template <class T, class U>
bool operator!=(const STLcustomalloc<T>&, const STLcustomalloc<U>&);


template<class T, class U>
inline bool operator==(const STLcustomalloc<T>&, const STLcustomalloc<U>&)
{
	return false;
}

template<class T, class U>
inline bool operator!=(const STLcustomalloc<T>& lhs, const STLcustomalloc<U>& rhs)
{
	return !(lhs==rhs);
}

template<typename T>
T * STLcustomalloc<T>::allocate(std::size_t n)
{
#ifdef SHOW_ALLOC_MSG
	std::cout << "ALLOCATED VIA MY CUSTOM ALLOCATOR!!!!!!!\n";
#endif
	T* ptr = (T*) (m->malloc(n * sizeof(T)));
	return ptr;
}

template<typename T>
void STLcustomalloc<T>::deallocate(T * p, std::size_t n)
{
	m->free((char*)p);
}

template<typename T>
STLcustomalloc<T>::STLcustomalloc()
{
	m = new MemoryManager();
}

template<typename T>
STLcustomalloc<T>::STLcustomalloc(std::size_t n)
{
	m = new MemoryManager(n*sizeof(T) + KB_64);
}

#endif //!__STL_CUSTOM_ALLOCATOR_H__

template<typename T>
template<class U>
STLcustomalloc<T>::STLcustomalloc(const STLcustomalloc<U>& other) : m(other.m)
{}
