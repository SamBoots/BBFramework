#pragma once
#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)

#include "Dynamic_Array.h"
#include "Utils/Math.h"

#include <vector>

std::vector<int> k;

TEST(Dynamic_ArrayDataStructure, Dynamic_push_reserve)
{
	constexpr const size_t initialSize = 8;
	constexpr const size_t samples = 512;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::Dynamic_Array<size2593bytes, BB::FreeListAllocator_t> t_Array(t_Allocator, initialSize);

	size_t t_RandomValues[samples]{};

	for (size_t i = 0; i < samples; i++)
	{
		t_RandomValues[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}

	//Cache the current capacity since we will go over it. 
	const size_t t_OldSize = t_Array.capacity();
	for (size_t i = 0; i < initialSize * BB::Dynamic_Array_Specs::overAllocateMultiplier; i++)
	{
		size2593bytes t_Object;
		t_Object.value = t_RandomValues[i];
		t_Array.push_back(t_Object);
	};

	//Test all results before resize
	for (size_t i = 0; i < t_Array.size(); i++)
	{
		EXPECT_EQ(t_Array[i].value, t_RandomValues[i]) << "Dynamic array value is wrong, something went bad.";
	}

	//
	EXPECT_EQ(t_OldSize, t_Array.capacity()) << "Dynamic array resized while it shouldn't!";
	
	size2593bytes t_LimitObject;
	t_LimitObject.value = t_RandomValues[t_Array.size() + 1];
	t_Array.push_back(t_LimitObject);

	EXPECT_NE(t_OldSize, t_Array.capacity()) << "Dynamic array did not resize, test might be unaccurate and needs to be updated.";

	//now just fill the entire thing up.
	for (size_t i = t_Array.size(); i < samples; i++)
	{
		size2593bytes t_Object;
		t_Object.value = t_RandomValues[i];
		t_Array.push_back(t_Object);
	};

	for (size_t i = 0; i < samples; i++)
	{
		EXPECT_EQ(t_Array[i].value, t_RandomValues[i]) << "Dynamic array value is wrong, something went bad.";
	}
}

TEST(Dynamic_ArrayDataStructure, Dynamic_Array_push_reserve)
{
	constexpr const size_t initialSize = 8;
	constexpr const size_t pushSize = 128;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	BB::FreeListAllocator_t t_Allocator(1024 * 1024 * 2);

	BB::Pool<size2593bytes, BB::FreeListAllocator_t> t_Array(t_Allocator, initialSize);

	size_t t_RandomValues[pushSize]{};
	size2593bytes* t_SizeArray[pushSize]{};

	for (size_t i = 0; i < pushSize; i++)
	{
		t_RandomValues[i] = static_cast<size_t>(BB::Utils::RandomUInt());

	}
}