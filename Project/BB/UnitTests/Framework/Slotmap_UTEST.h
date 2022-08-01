#pragma once
#include "../TestValues.h"
#include "Storage/Slotmap.h"

TEST(Slotmap_Datastructure, Slotmap_Insert_Erase_Iterator)
{
	constexpr const size_t samples = 128;

	//32 MB alloactor.
	const size_t allocatorSize = BB::mbSize * 32;
	//BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::Slotmap<size2593bytesObj> t_Map;

	{
		size2593bytesObj t_Value{};
		t_Value.value = 500;
		BB::SlotmapID t_ID = t_Map.insert(t_Value);
		ASSERT_EQ(t_Map.find(t_ID).value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.erase(t_ID);

		//try inserting again after an deletion.
		t_ID = t_Map.insert(t_Value);
		ASSERT_EQ(t_Map.find(t_ID).value, t_Value.value) << "Wrong element was likely grabbed.";

		t_Map.erase(t_ID);
	}

	size2593bytesObj t_RandomKeys[samples]{};
	BB::SlotmapID t_IDs[samples]{};
	for (size_t i = 0; i < samples; i++)
	{
		t_RandomKeys[i] = static_cast<size_t>(BB::Random::Random());
	}

	for (size_t i = 0; i < samples; i++)
	{
		t_IDs[i] = t_Map.insert(t_RandomKeys[i]);
	}


	size_t t_Count = 0;
	for (auto t_It = t_Map.begin(); t_It < t_Map.end(); t_It++)
	{
		ASSERT_EQ(t_It->value, t_RandomKeys[t_Count++].value) << "Wrong element was likely grabbed.";
	}
	ASSERT_EQ(samples, t_Count) << "Iterator went over the sample amount.";

	for (size_t i = 0; i < samples; i++)
	{
		t_Map.erase(t_IDs[i]);
	}

	for (size_t i = 0; i < samples; i++)
	{
		t_IDs[i] = t_Map.insert(t_RandomKeys[i]);
	}

	t_Count = 0;
	for (auto t_It = t_Map.begin(); t_It < t_Map.end(); t_It++)
	{
		ASSERT_EQ(t_It->value, t_RandomKeys[t_Count++].value) << "Wrong element was likely grabbed.";
	}
	ASSERT_EQ(samples, t_Count) << "Iterator went over the sample amount.";
}