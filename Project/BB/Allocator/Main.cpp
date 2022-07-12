// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <gtest/gtest.h>
#include "OS/OSDevice.h"
#include "Storage/Pool_UTEST.h"
#include "Storage/Dynamic_Array_UTEST.h"
#include "Storage/Hashmap_UTEST.h"
#include "Allocators/MemoryArena_UTEST.h"
#include "Allocators/Allocators_UTEST.h"
#include "Utils/Slice_UTEST.h"

int main()
{

	testing::InitGoogleTest();
	RUN_ALL_TESTS();

	BB::AppOSDevice().CreateOSWindow(BB::OS_WINDOW_STYLE::MAIN, 250, 200, 250, 200, "Memory Studies Window Main");

	BB::AppOSDevice().CreateOSWindow(BB::OS_WINDOW_STYLE::CHILD, 100, 100, 250, 50, "Memory Studies window1");

	BB::AppOSDevice().CreateOSWindow(BB::OS_WINDOW_STYLE::CHILD, 150, 100, 250, 100, "Memory Studies window2");

	while (BB::AppOSDevice().ProcessMessages())
	{

	}

	return 0;
}