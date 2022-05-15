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

	system("pause");
}

