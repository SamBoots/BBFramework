#pragma once
#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)

#include "Dynamic_Array.h"
#include "Utils/Math.h"

TEST(Dynamic_ArrayDataStructure, Dynamic_push_reserve)
{
	constexpr const size_t initialSize = 8;
	constexpr const size_t samples = initialSize * BB::Dynamic_Array_Specs::overAllocateMultiplier;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::Dynamic_Array<size2593bytes, BB::FreeListAllocator_t> t_Array(t_Allocator);
	EXPECT_EQ(t_Array.capacity(), BB::Dynamic_Array_Specs::multipleValue);

	//Allocate an object without having allocated memory, this must be valid.
	{
		constexpr const size_t testvalue = 123456;

		size2593bytes t_Object;
		t_Object.value = testvalue;
		t_Array.push_back(t_Object);
		EXPECT_EQ(t_Array[0].value, testvalue);
		//Real test starts now, so remove latest pushed variable.
		t_Array.pop();
	}
	size_t t_OldCapacity = t_Array.capacity();

	EXPECT_NE(t_OldCapacity, initialSize * BB::Dynamic_Array_Specs::overAllocateMultiplier) << "Just allocating one element seems to be enough for the test, this is wrong and might indicate an unaccurate test.";
	t_Array.reserve(initialSize);
	//Because of overallocationmultiplier this should reserve some.
	EXPECT_NE(t_OldCapacity, initialSize * BB::Dynamic_Array_Specs::overAllocateMultiplier) << "Reserve overallocates while reserve should be able to do that!";

	size_t t_RandomValues[samples]{};

	for (size_t i = 0; i < samples; i++)
	{
		t_RandomValues[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}

	//Cache the current capacity since we will go over it. 
	t_OldCapacity = t_Array.capacity();
	for (size_t i = 0; i <  -1 + initialSize; i++)
	{
		size2593bytes t_Object;
		t_Object.value = t_RandomValues[i];
		t_Array.push_back(t_Object);
	};

	//Test all results before doing a resize event
	for (size_t i = 0; i < t_Array.size(); i++)
	{
		EXPECT_EQ(t_Array[i].value, t_RandomValues[i]) << "Dynamic array value is wrong, something went bad before a resize event.";
	}
	

	EXPECT_EQ(t_OldCapacity, t_Array.capacity()) << "Dynamic array resized while it shouldn't!";
	
	size2593bytes t_LimitObject;
	t_LimitObject.value = t_RandomValues[t_Array.size()];
	t_Array.push_back(t_LimitObject);

	t_OldCapacity = t_Array.capacity();
	EXPECT_EQ(t_Array[t_Array.size() - 1].value, t_LimitObject.value) << "Dynamic Array resize event went badly.";

	//Empty the array to prepare for the next test.
	t_Array.empty();

	//Reserve enough for the entire test.
	t_Array.reserve(samples);

	EXPECT_EQ(t_Array.capacity(), samples) << "Dynamic Array's multiple seems to be changed or bad, this might indicate an unaccurate test.";

	//cache capacity so that we can compare if more was needed later on.
	t_OldCapacity = t_Array.capacity();

	//now just fill the entire thing up.
	for (size_t i = 0; i < -1 + samples; i++)
	{
		size2593bytes t_Object;
		t_Object.value = t_RandomValues[i];
		t_Array.push_back(t_Object);
	};

	for (size_t i = 0; i < -1 + samples; i++)
	{
		EXPECT_EQ(t_Array[i].value, t_RandomValues[i]) << "Dynamic array value is wrong, something went bad after the resize events.";
	}

	EXPECT_EQ(t_OldCapacity, t_Array.capacity()) << "Dynamic array capacity has been resized at the end while enough should've been reserve.";
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

	//Cache the current capacity since we will go over it. 
	//size_t t_OldSize = t_Array.capacity();
}