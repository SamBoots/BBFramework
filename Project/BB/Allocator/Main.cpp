// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <assert.h>

#include <chrono>

#include <gtest/gtest.h>

#include "Storage/Dynamic_Array.h"

std::chrono::high_resolution_clock timer;
std::chrono::high_resolution_clock::time_point timerStart;

using ms_t = std::chrono::duration<float, std::milli>;

constexpr const size_t testcases = 100000;
int main()
{
	testing::InitGoogleTest();
	RUN_ALL_TESTS();

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

	system("pause");
}

