// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <gtest/gtest.h>
#include "OS/OSDevice.h"

int main()
{
	testing::InitGoogleTest();
	RUN_ALL_TESTS();

	BB::AppOSDevice().CreateOSWindow(200, 200, 640, 480, "Memory Studies window");

	
	while (BB::AppOSDevice().ProcessMessages())
	{

	}

	return 0;
}

