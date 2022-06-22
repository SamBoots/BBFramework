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

int main()
{

	testing::InitGoogleTest();
	RUN_ALL_TESTS();

#ifndef _UNIX
	BB::AppOSDevice().CreateOSWindow(200, 200, 640, 480, "Memory Studies window");
	BB::framework_handle handle = FRAMEWORK_NULL_HANDLE;

	while (BB::AppOSDevice().ProcessMessages())
	{

	}
#else

	while (true)
	{

	}
#endif _UNIX
	return 0;
}

