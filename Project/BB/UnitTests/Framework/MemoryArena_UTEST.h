#pragma once
#include "../TestValues.h"
#include "Allocators/AllocTypes.h"

TEST(GlobalDebugAllocator, Macro_Test)
{
	size2593bytesObj* t_TestObj = BB::BBglobalalloc<size2593bytesObj>();
	t_TestObj->value = 2;
	
	EXPECT_EQ(t_TestObj->value, 2) << "BBglobalalloc didn't allocate correctly.";
	BB::BBglobaldestroy<size2593bytesObj>(t_TestObj);

	constexpr const size_t ArraySamples = 165;
	size2593bytesObj* t_TestObjArr = BB::BBglobalallocarray<size2593bytesObj>(ArraySamples);

	for (size_t i = 0; i < ArraySamples; i++)
	{
		t_TestObjArr[i].value = i * 2;
	}

	for (size_t i = 0; i < ArraySamples; i++)
	{
		EXPECT_EQ(t_TestObjArr[i].value, i * 2) << "BBglobalallocarray didn't allocate correctly.";
	}

	BB::BBglobaldestroyarray<size2593bytesObj>(t_TestObjArr);
}

#ifdef _DEBUG
TEST(MemoryAllocator_MemoryArena, COUNT_MEMORYTRACKER)
{
	//We use a freelist for the test since it allows for easy allocations and deallocations.
	constexpr const size_t ALLOCATORSIZE = 41943040;

	constexpr const size_t TRACKAMOUNT = 512;
	constexpr const size_t MAX_RANDOM_VALUE = 4096; //4 KB.
	constexpr const size_t MIN_RANDOM_VALUE = 1024; //1 KB.

	struct MemoryTrackInstance
	{
		void* ptr;
		size_t size;
	};

	struct MockMemoryArena : BB::FreeListAllocator_t
	{
		MockMemoryArena() : BB::FreeListAllocator_t(ALLOCATORSIZE) {}

		void CheckAllocExists(const MemoryTrackInstance& a_Instance)
		{
			ASSERT_EQ(allocationsDone, m_MemoryTrack.m_TrackingList.size()) << "Memory not correctly tracked, allocations done is not equal to the trackinglist.";

			auto t_It = m_MemoryTrack.m_TrackingList.find(BB::Pointer::Subtract(a_Instance.ptr, BB::MemoryDebugTools::BOUNDRY_FRONT));
			ASSERT_NE(t_It, m_MemoryTrack.m_TrackingList.end()) << "Memory allocation doesn't exist on the tracking list.";

			ASSERT_EQ(a_Instance.size + BB::MemoryDebugTools::BOUNDRY_BACK + BB::MemoryDebugTools::BOUNDRY_FRONT, t_It->second) << "Memory allocation doesn't share the size.";
		};

		void CheckAllocDoesntExists(const MemoryTrackInstance& a_Instance)
		{
			ASSERT_EQ(allocationsDone, m_MemoryTrack.m_TrackingList.size()) << "Memory not correctly tracked, allocations done is not equal to the trackinglist.";

			ASSERT_EQ(m_MemoryTrack.m_TrackingList.find(BB::Pointer::Subtract(a_Instance.ptr, BB::MemoryDebugTools::BOUNDRY_FRONT)), m_MemoryTrack.m_TrackingList.end()) << "Memory allocation exist on the tracking list while it shouldn't.";
		};

		void* MockAlloc(size_t a_Size, size_t a_Alignment)
		{
			allocationsDone++;
			return Alloc(a_Size, a_Alignment);
		}

		void MockFree(void* a_Ptr)
		{
			allocationsDone--;
			Free(a_Ptr);
		}

		size_t allocationsDone = 0;

	} t_MockArena{};

	MemoryTrackInstance t_TrackInstances[TRACKAMOUNT]{};

	for (size_t i = 0; i < TRACKAMOUNT; i++)
	{
		const size_t t_RandomAllocSize = BB::Random::Random(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);

		t_TrackInstances[i].size = t_RandomAllocSize;
		t_TrackInstances[i].ptr = t_MockArena.MockAlloc(t_RandomAllocSize, __alignof(t_RandomAllocSize));
	}

	//Check if the allocation exists in the tracking list.
	for (size_t i = 0; i < TRACKAMOUNT; i++)
	{
		t_MockArena.CheckAllocExists(t_TrackInstances[i]);
	}

	//now just dealloc the entire thing and check if the tracking still exists.
	for (size_t i = 0; i < TRACKAMOUNT; i++)
	{
		t_MockArena.MockFree(t_TrackInstances[i].ptr);
		t_MockArena.CheckAllocDoesntExists(t_TrackInstances[i]);
	}
}

#endif //_DEBUG