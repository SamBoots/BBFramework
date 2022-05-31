#pragma once
#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)

#include "Hashmap.h"
#include "Utils/Math.h"

TEST(Unordered_Map_Hashmap_Datastructure, Hashmap_Insert)
{
	constexpr const uint32_t samples = 256;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::UM_HashMap<size2593bytes, uint32_t, BB::FreeListAllocator_t> t_Map(t_Allocator);

	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = 124;
		t_Map.Insert(t_Value, t_Key);

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_Key);
		ASSERT_EQ(t_Map.Find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}

	uint32_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}
	
	for (uint32_t i = 0; i < samples; i++)
	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = t_RandomKeys[i];
		t_Map.Insert(t_Value, t_Key);

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_Key);
		ASSERT_EQ(t_Map.Find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}
}

TEST(Open_Addressing_Hashmap_Datastructure, Hashmap_Insert)
{
	constexpr const uint32_t samples = 256;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::OL_HashMap<size2593bytes, uint32_t, BB::FreeListAllocator_t> t_Map(t_Allocator);

	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = 124;
		t_Map.Insert(t_Value, t_Key);

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_Key);
		ASSERT_EQ(t_Map.Find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}

	uint32_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}

	for (uint32_t i = 0; i < samples; i++)
	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = t_RandomKeys[i];
		t_Map.Insert(t_Value, t_Key);

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_Key);
		ASSERT_EQ(t_Map.Find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}
}


TEST(Open_Addressing_Hashmap_Datastructure, Hashmap_Insert)
{
	constexpr const uint32_t samples = 256;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::OL_HashMap<size2593bytes, uint32_t, BB::FreeListAllocator_t> t_Map(t_Allocator);

	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = 124;
		t_Map.Insert(t_Value, t_Key);

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_Key);
		ASSERT_EQ(t_Map.Find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}

	uint32_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}

	for (uint32_t i = 0; i < samples; i++)
	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = t_RandomKeys[i];
		t_Map.Insert(t_Value, t_Key);

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_Key);
		ASSERT_EQ(t_Map.Find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}
}

#include <chrono>
#include <unordered_map>

TEST(Hashmap, Hashmap_Speedtest_Insertion)
{
	constexpr const uint32_t samples = 256;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//Just a giant allocator required for this test.
	const size_t allocatorSize = BB::gbSize * 4;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);
	
	
	
	auto t_Timer = std::chrono::high_resolution_clock::now();


}