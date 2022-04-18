// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <assert.h>

#include <chrono>

#include "Framework/MemoryArena.h"

std::chrono::high_resolution_clock timer;
std::chrono::high_resolution_clock::time_point timerStart;

using ms_t = std::chrono::duration<float, std::milli>;

typedef BB::MemoryArena<BB::allocators::LinearAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTagging> unsafeLinearAllocator_t;
typedef BB::MemoryArena<BB::allocators::FreelistAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTagging> unsafeFreeListAllocator_t;


constexpr const size_t testcases = 1000000;
int main()
{
	unsafeLinearAllocator_t linearAlloc(testcases * 8);
	unsafeFreeListAllocator_t freelistAlloc((testcases * 8) * 2 + sizeof(BB::allocators::FreelistAllocator::AllocHeader));
	unsafeFreeListAllocator_t freelistAllocArray((testcases * 8) * 20);


	std::cout << "\n \n" << "SINGLE ALLOCATION TESTS" << "\n";

	{
		timerStart = timer.now();

		for (size_t i = 0; i < testcases; i++)
		{
			int* index = new int();
			delete index;
		}
		auto timerStop = timer.now();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " STANDARD NEW: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	{
		timerStart = timer.now();

		for (size_t i = 0; i < testcases; i++)
		{
			int* index = BB::AllocNew<int>(linearAlloc);
		}
		auto timerStop = timer.now();
		linearAlloc.Clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " LINEAR ALLOCATOR: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	{
		timerStart = timer.now();

		for (size_t i = 0; i < testcases; i++)
		{
			int* index = BB::AllocNew<int>(freelistAllocArray);
		}
		auto timerStop = timer.now();
		linearAlloc.Clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " FREELIST ALLOCATOR: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}

	std::cout << "\n \n" << "ARRAY ALLOCATION TESTS" << "\n";

	{
		timerStart = timer.now();

		int* index = new int[testcases]();
		auto timerStop = timer.now();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << "  STANDARD NEW: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}

	{
		timerStart = timer.now();

		int* index = BB::AllocNewArray<int>(linearAlloc, testcases);
		auto timerStop = timer.now();

#ifdef _DEBUG
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

		int* index = BB::AllocNewArray<int>(freelistAlloc, testcases);
		auto timerStop = timer.now();

#ifdef _DEBUG
		int number = 5;
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

		linearAlloc.Clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " FREELIST ALLOCATOR: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}

	system("pause");
}

