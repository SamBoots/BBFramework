// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <assert.h>

#include <chrono>

#include "Framework/Allocators.h"

std::chrono::high_resolution_clock timer;
std::chrono::high_resolution_clock::time_point timerStart;

using ms_t = std::chrono::duration<float, std::milli>;

constexpr const size_t testcases = 1000000;
int main()
{
	BB::memory::LinearAllocator linearAlloc(testcases * 8);

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
		linearAlloc.clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " LINEAR ALLOCATOR: " << testcases << " INDIVIDUAL ALLOCATIONS \n";
	}


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

		linearAlloc.clear();
		std::cout << std::chrono::duration_cast<ms_t>(timerStop - timerStart).count() << " LINEAR ALLOCATOR: " << testcases << " SINGLE ARRAY ALLOCATION \n";
	}


}

