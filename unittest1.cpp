#include "stdafx.h"
#include "CppUnitTest.h"
#include "../MemoryAllocator/MemoryAllocator.h"
#include "../MemoryAllocator/MemoryAllocator.cpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

BEGIN_TEST_MODULE_ATTRIBUTE()
	TEST_MODULE_ATTRIBUTE(L"Project", L"UnitTests")
	TEST_MODULE_ATTRIBUTE(L"Date", L"24/01/2016")
END_TEST_MODULE_ATTRIBUTE()

TEST_MODULE_INITIALIZE(ModuleInitialize)
{
	Logger::WriteMessage("Entered ModuleInitialize");
}

TEST_MODULE_CLEANUP(ModuleCleanup)
{
	Logger::WriteMessage("Entering ModuleCleanup");
}


namespace Test_MemoryAllocator
{		
	TEST_CLASS(Test_MA)
	{
	public:
		
		Test_MA()
		{
			Logger::WriteMessage("Entering TestMA::TestMA()");
		}

		~Test_MA()
		{
			Logger::WriteMessage("Entering TestMA::~TestMA()");
		}

		TEST_METHOD(TestMalloc)
		{
			Logger::WriteMessage("Testing TestMalloc()");
			MemoryAllocator m;
			int* x = (int*)m.malloc(sizeof(int) * 20);

		}

		TEST_METHOD(TestMallocThrow)
		{
			Logger::WriteMessage("Testing TestMallocThrow()");
			MemoryAllocator m(1);
			//Expect bad alloc exception
			try {
				int*x = (int*)m.malloc(SIZE_MAX / 2);
			}
			catch (std::bad_alloc&) {
				//It's fine
			}
		}
		
		TEST_METHOD(TestMallocNoThrow)
		{
			Logger::WriteMessage("Testing TestMallocNoThrow()");

			MemoryAllocator m(1);
			int* x = (int*)m.malloc(SIZE_MAX / 2, NO_THROW);
		
			//Why can't I Assert::AreEqual(x, nullptr) ???
			Assert::AreEqual((int)x, 0);
		}

		TEST_METHOD(TestCreateAllocator)
		{
			Logger::WriteMessage("Testing TestCreateAllocator()");

			size_t size = 10;
			MemoryAllocator m(size);

		}

		TEST_METHOD(TestCreateAllocatorOverflow)
		{
			Logger::WriteMessage("Testing TestCreateAllocatorOverflow()");


			//In the order of 2TB+
			//It's not expected this test to be run on that kind of machine
			try {
				MemoryAllocator m(SIZE_MAX /2);
			}
			catch (std::bad_alloc&)
			{
				//do nothing
			}
				
		}

		TEST_METHOD(TestStoreAndRetrieve)
		{
			Logger::WriteMessage("Testing TestStoreAndRetrieve()");


			size_t size = 10;
			MemoryAllocator m(size);

			int *pArr = (int*)m.malloc(sizeof(int));

			*pArr = 10;

			Assert::AreEqual(*pArr, 10);
		}

		TEST_METHOD(TestStoreAndDeleteManyTimes)
		{
			Logger::WriteMessage("Testing TestStoreAndDeleteManyTimes()");

			size_t size = 100;
			MemoryAllocator m(size);

			for (int i = 0; i < 10'000; i++)
			{
				int *pArr = (int*)m.malloc(sizeof(int));
				m.free((char*)pArr);
			}
		}

		TEST_METHOD(TestFreeFunction)
		{
			Logger::WriteMessage("Testing TestFreeFunction()");

			//Make allocator with 1mb of memory
			MemoryAllocator m(1);

			//Allocate and free 1 million times
			for (int i = 0; i < 1'000'000; i++)
			{
				int *pArr = (int*)m.malloc(sizeof(int));
				m.free((char*)pArr);
			}
		}
		TEST_METHOD(TestManyRetrieves)
		{
			Logger::WriteMessage("Testing TestManyRetrieves()");

			MemoryAllocator m;

			int* arr = new int[10000];
			int** arr2 = new int*[10'000];
			int tmp;
			for (int i = 0; i < 10'000; i++)
			{
				tmp = rand() % 5000;
				arr[i] = tmp;
				arr2[i] = (int*)m.malloc(sizeof(int));
				*(arr2[i]) = tmp;
			}

			for (int i = 0; i < 10'000; i++)
			{
				Assert::AreEqual(*(arr2[i]), arr[i]);
			}
		}
		TEST_METHOD(TestMyMallocSpeed)
		{
			Logger::WriteMessage("Testing TestMyMallocSpeed()");
			
			MemoryAllocator m(100);

			//With 1'000'000 it runs for 1 seconds on Intel m core 5Y71 
			//(1.2GHz base frequency, up to 2.9GHz with turbo boost)
			// ~ 3 000 000 allocations and 3 000 000 frees per second
			// ~ 6 000 000 operations per second
			for (int i = 0; i < 3'000'000; i++)
			{
				char* pArr = m.malloc(rand() % 500);
				m.free(pArr);
				char* pArr1 = m.malloc(rand() % 10);
				m.free(pArr1);
				char* pArr2 = m.malloc(500 + rand() % 500);
				m.free(pArr2);
			}
			char* pArr3 = (char*)m.malloc(500000);
			m.free(pArr3);

		}
		TEST_METHOD(TestMany)
		{
			MemoryAllocator m(3);
			int** arr = new int*[11'000];
			for (int i = 0; i < 10'000; i++)
				arr[i] = (int*)m.malloc(sizeof(int));
			for (int i = 0; i < 10'000; i++)
				m.free((char*)arr[i]);

			char* pArr = m.malloc(50'000);

		}
		TEST_METHOD(TestSpeedStdMalloc)
		{
			Logger::WriteMessage("Testing TestSpeedStdMalloc()");

			//Test the standard malloc function for comparison
			for (int i = 0; i < 1'000'000; i++)
			{
				char* pArr = (char*)malloc(rand() % 500);
				free(pArr);
				char* pArr1 = (char*)malloc(rand() % 10);
				free(pArr1);
				char* pArr2 = (char*)malloc(500 + rand() % 500);
				free(pArr2);
			}
		}

		TEST_METHOD(TestSpeedStdNew)
		{
			Logger::WriteMessage("Testing TestSpeedStdNew()");

			//Test the standard new function for comparison
			for (int i = 0; i < 1'000'000; i++)
			{
				char* pArr = new char[(rand() % 500)];
				delete[] pArr;
				char* pArr1 = new char[(rand() % 10)];;
				delete[] pArr1;
				char* pArr2 = new char[500 + (rand() % 500)];
				delete[] pArr2;
			}
		}

		TEST_METHOD(TestCoalescence)
		{
			Logger::WriteMessage("Testing TestCoalescence()");

			MemoryAllocator m(10);
			int const MAX = 500;
			int** arr = new int*[MAX];
			for (int i = 0; i < MAX; i++) {
				arr[i] = (int*)m.malloc(MAX);
			}

			//Free them after allocating all
			//If in the end there's only 1 element in the free list
			//they have been coalescenced
			for (int i = 0; i < MAX; i++) {
				m.free((char*)arr[i]);
			}

			Assert::AreEqual(m.count_in_flist(), 1);

		}

		TEST_METHOD(TestSpliting)
		{
			Logger::WriteMessage("Testing TestSplitting()");

			MemoryAllocator m(1);
			//Almost 1 full megabyte
			char* a = m.malloc(1 * 1024 * 1024 - 2000);
			m.free(a);

			//If splitting doesn't work these mallocs should fail because of
			//unsufficient memory
			char* b = m.malloc(20000);
			char* c = m.malloc(50000);
			char* d = m.malloc(100000);
			char* e = m.malloc(100000);
		}
	};
}