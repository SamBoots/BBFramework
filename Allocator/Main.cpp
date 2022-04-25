// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <assert.h>

#include <chrono>

#include "Framework/Storage/Dynamic_Array.h"

std::chrono::high_resolution_clock timer;
std::chrono::high_resolution_clock::time_point timerStart;

using ms_t = std::chrono::duration<float, std::milli>;

constexpr const size_t testcases = 100000;
int main()
{
	BB::unsafeLinearAllocator_t linearAlloc(testcases * 8);
	BB::unsafeFreeListAllocator_t freelistAlloc((testcases * 8) * sizeof(BB::allocators::FreelistAllocator::AllocHeader));
	BB::unsafePoolAllocator_t poolAllocator(sizeof(size_t), testcases, __alignof(size_t));

	std::cout << "\n \n" << "SINGLE ALLOCATION TESTS" << "\n";

	{
		timerStart = timer.now();

		for (size_t i = 0; i < testcases; i++)
		{
			size_t* index = new size_t();
			delete index;
		}
		auto timerStop = timer.now();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " STANDARD NEW: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	{
		timerStart = timer.now();

		for (size_t i = 0; i < testcases; i++)
		{
			size_t* index = BB::AllocNew<size_t>(linearAlloc);
		}
		auto timerStop = timer.now();
		linearAlloc.Clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " LINEAR ALLOCATOR: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	{
		timerStart = timer.now();

		for (size_t i = 0; i < testcases; i++)
		{
			size_t* index = BB::AllocNew<size_t>(freelistAlloc);
			BB::Free(freelistAlloc, index);
		}
		auto timerStop = timer.now();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " FREELIST ALLOCATOR: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	{
		timerStart = timer.now();
		for (size_t i = 0; i < testcases; i++)
		{
			size_t* index = BB::AllocNew<size_t>(poolAllocator);
			BB::Free(poolAllocator, index);
		}

		auto timerStop = timer.now();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " POOL ALLOCATOR: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	std::cout << "\n \n" << "ARRAY ALLOCATION TESTS" << "\n";

	{
		timerStart = timer.now();

		size_t* index = new size_t[testcases]();
		delete[] index;

		auto timerStop = timer.now();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << "  STANDARD NEW: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}

	{
		timerStart = timer.now();

		size_t* index = BB::AllocNewArray<size_t>(linearAlloc, testcases);
		auto timerStop = timer.now();

#ifdef DEBUG
		int number = 5;
		for (size_t i = 0; i < testcases; i++)
		{
			index[i] = number;
			number += 2;
		}

		for (size_t i = 0; i < testcases; i++)
		{
			std::cout << "Linear allocated array position: " << i << " has value: " << index[i] << "\n";
		}
#endif
		linearAlloc.Clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " LINEAR ALLOCATOR: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}

	{
		timerStart = timer.now();

		size_t* index = BB::AllocNewArray<size_t>(freelistAlloc, testcases);
		auto timerStop = timer.now();

#ifdef DEBUG
		size_t number = 5;
		for (size_t i = 0; i < testcases; i++)
		{
			index[i] = number;
			number += 2;
		}

		for (size_t i = 0; i < testcases; i++)
		{
			std::cout << "Freelist allocated array position: " << i << " has value: " << index[i] << "\n";
		}
#endif
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " FREELIST ALLOCATOR: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}

	{
		timerStart = timer.now();

		size_t* index = BB::AllocNewArray<size_t>(poolAllocator, testcases);
		auto timerStop = timer.now();

#ifdef DEBUG
		size_t number = 5;
		for (size_t i = 0; i < testcases; i++)
		{
			index[i] = number;
			number += 2;
		}

		for (size_t i = 0; i < testcases; i++)
		{
			std::cout << "Pool allocated array position: " << i << " has value: " << index[i] << "\n";
		}
#endif
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " POOL ALLOCATOR: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}


	std::cout << "\n \n" << "DATA STORAGE TESTS" << "\n";

	{
		timerStart = timer.now();
		BB::Dynamic_Array<size_t> dynamic_Array;

		for (size_t i = 0; i < 8; i++)
		{
			size_t numb = (i + 2) * 2;
			dynamic_Array.push_back(numb);
		}

		auto timerStop = timer.now();

#ifdef _DEBUG
		for (size_t i = 0; i < 8; i++)
		{
			std::cout << "Pool allocated array position: " << i << " has value: " << dynamic_Array[i] << "\n";
		}
#endif
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " DYNAMIC_ARRAY: " << testcases << "INSERTING SINGLE ELEMENTS \n";
	}

	//system("pause");
}

