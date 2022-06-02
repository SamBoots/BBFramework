#pragma once
#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)

#include "Hashmap.h"
#include "Utils/Math.h"

TEST(Hashmap_Datastructure, UM_Hashmap_Insert)
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

TEST(Hashmap_Datastructure, OL_Hashmap_Insert)
{
	constexpr const uint32_t samples = 2024;
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
		t_RandomKeys[i] = (i + 1) * 10;
	}

	//Only test a quarter of the samples for just inserting and removing.
	for (uint32_t i = 0; i < samples / 4; i++)
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

	//FIll the map to double it's size and test if a resize works.
	for (uint32_t i = 0; i < samples; i++)
	{
		size2593bytes t_Value{};
		t_Value.value = t_RandomKeys[i] << 2;
		uint32_t t_Key = t_RandomKeys[i];
		t_Map.Insert(t_Value, t_Key);


	}
	//Now check it
	for (uint32_t i = 0; i < samples; i++)
	{
		uint32_t t_Key = t_RandomKeys[i];

		ASSERT_NE(t_Map.Find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.Find(t_Key)->value, t_Key << 2) << "Wrong element was likely grabbed.";

		t_Map.Remove(t_RandomKeys[i]);
		ASSERT_EQ(t_Map.Find(t_RandomKeys[i]), nullptr) << "Element was found while it should've been deleted.";
	}
}

#include <chrono>
#include <unordered_map>

TEST(Hashmap_Datastructure, Hashmap_Speedtest)
{
	//Some tools for the speed test.
	typedef std::chrono::duration<float, std::milli> ms;
	constexpr const float MILLITIMEDIVIDE = 1 / 1000.f;

	constexpr const size_t samples = 512;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//Just a giant allocator required for this test.
	const size_t allocatorSize = BB::gbSize * 4;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	//all the maps
	std::unordered_map<uint32_t, size2593bytes> t_UnorderedMap;
	BB::OL_HashMap<size2593bytes, size_t, BB::FreeListAllocator_t> t_OL_Map(t_Allocator);

	//The samples we will use as an example.
	size_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}

	
	{	
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytes t_Insert;
			t_Insert.value = t_RandomKeys[i];
			t_UnorderedMap.insert(std::make_pair(i, t_Insert));
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map emplace speed with: " << samples <<
			" elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytes t_Insert;
			t_Insert.value = t_RandomKeys[i];
			t_OL_Map.Insert(t_Insert, i);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map emplace speed with: " << samples <<
			" elements took this much MS: " << t_OLMapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_UnorderedMap.erase(i);
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map remove speed with: " << samples <<
			" elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_OL_Map.Remove(i);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map remove speed with: " << samples <<
			" elements took this much MS: " << t_OLMapSpeed << "\n";
	}
}