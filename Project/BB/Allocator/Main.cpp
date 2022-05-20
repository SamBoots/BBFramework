// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <gtest/gtest.h>
#include "OS/OSDevice.h"
#include "Storage/Pool.h"

int main()
{
	BB::AppOSDevice().CreateOSWindow(200, 200, 640, 480, "Memory Studies window");

	testing::InitGoogleTest();
	RUN_ALL_TESTS();
	
	while (BB::AppOSDevice().ProcessMessages())
	{

	}

	return 0;
}

