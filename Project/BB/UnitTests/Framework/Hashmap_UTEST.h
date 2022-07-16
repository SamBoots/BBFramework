#pragma once
#include "../TestValues.h"
#include "Storage/Hashmap.h"

TEST(Hashmap_Datastructure, UM_Hashmap_Insert)
{
	constexpr const size_t samples = 256;

	//32 MB alloactor.
	const size_t allocatorSize = BB::mbSize * 32;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::UM_HashMap<size_t, size2593bytesObj> t_Map(t_Allocator);

	{
		size2593bytesObj t_Value{};
		t_Value.value = 500;
		size_t t_Key = 124;
		t_Map.insert(t_Key, t_Value);

		ASSERT_NE(t_Map.find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.remove(t_Key);
		ASSERT_EQ(t_Map.find(t_Key), nullptr) << "Element was found while it should've been deleted.";
	}

	uint32_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Random::Random());
	}
	
	for (uint32_t i = 0; i < samples; i++)
	{
		size2593bytesObj t_Value{};
		t_Value.value = 500;
		size_t t_Key = t_RandomKeys[i];
		t_Map.insert(t_Key, t_Value);

		ASSERT_NE(t_Map.find(t_Key), nullptr) << "Cannot find the element while it was added!";
		ASSERT_EQ(t_Map.find(t_Key)->value, t_Value.value) << "Wrong element was likely grabbed.";
	}
}

TEST(Hashmap_Datastructure, OL_Hashmap_Insert)
{
	constexpr const uint32_t samples = 4096;

	//32 MB alloactor.
	const size_t allocatorSize = BB::mbSize * 32;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::OL_HashMap<size_t, size2593bytesObj> t_Map(t_Allocator);
	t_Map.reserve(samples);
	{
		size2593bytesObj t_Value{};
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
		size2593bytesObj t_Value{};
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
		size2593bytesObj t_Value{};
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
	}
}

#include <chrono>
#include <unordered_map>

TEST(Hashmap_Datastructure, Hashmap_Speedtest)
{
	//Some tools for the speed test.
	typedef std::chrono::duration<float, std::milli> ms;
	constexpr const float MILLITIMEDIVIDE = 1 / 1000.f;

	constexpr const size_t samples = 8192;

	//32 MB alloactor.
	const size_t allocatorSize = BB::mbSize * 32;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	//all the maps
	std::unordered_map<size_t, size2593bytesObj> t_UnorderedMap;
	BB::UM_HashMap<size_t, size2593bytesObj> t_UM_Map(t_Allocator);
	BB::OL_HashMap<size_t, size2593bytesObj> t_OL_Map(t_Allocator);

	t_UnorderedMap.reserve(samples);
	t_UM_Map.reserve(samples);
	t_OL_Map.reserve(samples);

	//The samples we will use as an example.
	size_t t_RandomKeys[samples]{};
	for (uint32_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Random::Random());
	}
	
	std::cout << "/-----------------------------------------/" << "\n";
#pragma region Insert_Test
	{	
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytesObj t_Insert{};
			t_Insert.value = i;
			t_UnorderedMap.emplace(std::make_pair(t_RandomKeys[i], t_Insert));
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map emplace speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytesObj t_Insert{};
			t_Insert.value = i;
			t_UM_Map.emplace(t_RandomKeys[i], t_Insert);
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map emplace speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_UMMapSpeed << "\n";
	}

	{
		
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			size2593bytesObj t_Insert{};
			t_Insert.value = i;
			t_OL_Map.emplace(t_RandomKeys[i], t_Insert.value);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map emplace speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_OLMapSpeed << "\n";
	}

#pragma endregion
	std::cout << "/-----------------------------------------/" << "\n";
#pragma region Lookup_Test

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_UnorderedMap.find(t_RandomKeys[i])->second.value, i) << "std unordered Hashmap couldn't find key " << t_RandomKeys[i];
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map lookup speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_UM_Map.find(t_RandomKeys[i])->value, i) << "UM Hashmap couldn't find key " << t_RandomKeys[i];
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map lookup speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_UMMapSpeed << "\n";
	}

	{

		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_OL_Map.find(t_RandomKeys[i])->value, i) << "OL Hashmap couldn't find key " << t_RandomKeys[i];
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map lookup speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_OLMapSpeed << "\n";
	}

#pragma endregion
	std::cout << "/-----------------------------------------/" << "\n";
#pragma region Lookup_Empty_Test

	constexpr const size_t EMPTY_KEY = 414141414141;
	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_UnorderedMap.find(EMPTY_KEY + i), t_UnorderedMap.end()) << "std unordered Hashmap found a key while it shouldn't exist." << t_RandomKeys[i];
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map lookup null speed with: " << samples <<
			" took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_UM_Map.find(EMPTY_KEY + i), nullptr) << "UM Hashmap found a key while it shouldn't exist." << t_RandomKeys[i];
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map lookup null speed with: " << samples <<
			" took this much MS: " << t_UMMapSpeed << "\n";
	}

	{

		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			EXPECT_EQ(t_OL_Map.find(EMPTY_KEY + i), nullptr) << "OL Hashmap found a key while it shouldn't exist." << t_RandomKeys[i];
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map lookup null speed with: " << samples <<
			" took this much MS: " << t_OLMapSpeed << "\n";
	}

#pragma endregion
	std::cout << "/-----------------------------------------/" << "\n";
#pragma region Removal_Test
	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//Unordered Map speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_UnorderedMap.erase(t_RandomKeys[i]);
		}
		auto t_Unordered_MapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "Unordered map remove speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_Unordered_MapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::UM speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_UM_Map.remove(t_RandomKeys[i]);
		}
		auto t_UMMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "UM map remove speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_UMMapSpeed << "\n";
	}

	{
		auto t_Timer = std::chrono::high_resolution_clock::now();
		//BB::OL speed.
		for (size_t i = 0; i < samples; i++)
		{
			t_OL_Map.remove(t_RandomKeys[i]);
		}
		auto t_OLMapSpeed = std::chrono::duration_cast<ms>(std::chrono::high_resolution_clock::now() - t_Timer).count() * MILLITIMEDIVIDE;
		std::cout << "OL map remove speed with: " << samples <<
			" 2593 byte elements took this much MS: " << t_OLMapSpeed << "\n";
	}
#pragma endregion
	std::cout << "/-----------------------------------------/" << "\n";
}