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

	BB::UM_HashMap<uint32_t, size2593bytes, BB::FreeListAllocator_t> t_Map(t_Allocator);

	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		uint32_t t_Key = 124;
		t_Map.insert(t_Key, t_Value);

		ASSERT_NE(t_Map.find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.remove(t_Key);
		ASSERT_EQ(t_Map.find(t_Key), nullptr) << "Element was found while it should've been deleted.";
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
		t_Map.insert(t_Key, t_Value);

		ASSERT_NE(t_Map.find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.remove(t_Key);
		ASSERT_EQ(t_Map.find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}
}

TEST(Hashmap_Datastructure, OL_Hashmap_Insert)
{
	constexpr const uint32_t samples = 4096;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::OL_HashMap<size_t, size2593bytes, BB::FreeListAllocator_t> t_Map(t_Allocator);
	t_Map.reserve(samples);
	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		size_t t_Key = 124;
		t_Map.insert(t_Key, t_Value);

		ASSERT_NE(t_Map.find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.remove(t_Key);
		ASSERT_EQ(t_Map.find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}

	size_t t_RandomKeys[samples]{};
	for (size_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = (i + 1) * 2;
	}

	//Only test a quarter of the samples for just inserting and removing.
	for (size_t i = 0; i < samples / 4; i++)
	{
		size2593bytes t_Value{};
		t_Value.value = 500;
		size_t t_Key = t_RandomKeys[i];
		t_Map.insert(t_Key, t_Value);

		ASSERT_NE(t_Map.find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.remove(t_Key);
		ASSERT_EQ(t_Map.find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}

	//FIll the map to double it's size and test if a resize works.
	for (size_t i = 0; i < samples; i++)
	{
		size2593bytes t_Value{};
		t_Value.value = t_RandomKeys[i] + 2;
		size_t t_Key = t_RandomKeys[i];
		t_Map.insert(t_Key, t_Value);
	}
	//Now check it
	for (size_t i = 0; i < samples; i++)
	{
		size_t t_Key = t_RandomKeys[i];

		EXPECT_NE(t_Map.find(t_Key), nullptr) << " Cannot find the element while it was added!";
		if (t_Map.find(t_Key) != nullptr)
			EXPECT_EQ(t_Map.find(t_Key)->value, t_Key + 2) << "element: " << i << " Wrong element was likely grabbed.";

		t_Map.remove(t_RandomKeys[i]);
		//EXPECT_EQ(t_Map.Find(t_RandomKeys[i]), nullptr) << "element: " << i << " Element was found while it should've been deleted.";
	}
}

#include <chrono>
#include <unordered_map>

TEST(Hashmap_Datastructure, Hashmap_Speedtest)
{
	//Some tools for the speed test.
	typedef std::chrono::duration<float, std::milli> ms;
	constexpr const float MILLITIMEDIVIDE = 1 / 1000.f;

	constexpr const size_t samples = 4096;
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//Just a giant allocator required for this test.
	const size_t allocatorSize = BB::gbSize * 4;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	//all the maps
	std::unordered_map<size_t, size2593bytes> t_UnorderedMap;
	BB::UM_HashMap<size_t, size2593bytes, BB::FreeListAllocator_t> t_UM_Map(t_Allocator);
	BB::OL_HashMap<size_t, size2593bytes, BB::FreeListAllocator_t> t_OL_Map(t_Allocator);

	t_UnorderedMap.reserve(4096);
	t_UM_Map.reserve(4096);
	t_OL_Map.reserve(4096);

	//The samples we will use as an example.
	size_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Utils::RandomUInt());
	}
	

#pragma region Insert_Test
	{	
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytes t_Insert;
			t_Insert.value = i;
			t_UnorderedMap.insert(std::make_pair(t_RandomKeys[i], t_Insert));
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map emplace speed with: " << samples <<
			" elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytes t_Insert;
			t_Insert.value = i;
			t_UM_Map.insert(t_RandomKeys[i], t_Insert);
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map emplace speed with: " << samples <<
			" elements took this much MS: " << t_UMMapSpeed << "\n";
	}

	{
		
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytes t_Insert;
			t_Insert.value = i;
			t_OL_Map.insert(t_RandomKeys[i], t_Insert);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map emplace speed with: " << samples <<
			" elements took this much MS: " << t_OLMapSpeed << "\n";
	}

#pragma endregion

#pragma region Lookup_Test

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_UnorderedMap.find(t_RandomKeys[i])->second.value, i);
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map lookup speed with: " << samples <<
			" elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_UM_Map.find(t_RandomKeys[i])->value, i);
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map lookup speed with: " << samples <<
			" elements took this much MS: " << t_UMMapSpeed << "\n";
	}

	{

		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_OL_Map.find(t_RandomKeys[i])->value, i);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map lookup speed with: " << samples <<
			" elements took this much MS: " << t_OLMapSpeed << "\n";
	}

#pragma endregion

#pragma region Removal_Test
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
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_UM_Map.remove(i);
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map remove speed with: " << samples <<
			" elements took this much MS: " << t_UMMapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_OL_Map.remove(i);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map remove speed with: " << samples <<
			" elements took this much MS: " << t_OLMapSpeed << "\n";
	}
#pragma endregion
}