#include "pch.h"
#include "MemoryArena.h"

#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)
#include "Utils/Math.h"
#include "Allocators.h"

TEST(MemoryAllocator_MemoryArena, COUNT_MEMORYTRACKER)
{
	//We use a freelist for the test since it allows for easy allocations and deallocations.
	typedef BB::MemoryArena<BB::allocators::FreelistAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::Count_MemoryTrack> LinearAllocatorWithTracking;
	constexpr const size_t ALLOCATORSIZE = 41943040;

	constexpr const size_t TRACKAMOUNT = 512;
	constexpr const size_t MAX_RANDOM_VALUE = 4096; //4 KB.
	constexpr const size_t MIN_RANDOM_VALUE = 1024; //1 KB.

	struct MemoryTrackInstance
	{
		void* ptr;
		size_t size;
	};

	struct MockMemoryArena : LinearAllocatorWithTracking
	{
		MockMemoryArena() : LinearAllocatorWithTracking(ALLOCATORSIZE) {}

		void CheckAllocExists(const MemoryTrackInstance& a_Instance)
		{
			ASSERT_EQ(allocationsDone, m_MemoryTrackPolicy.m_TrackingList.size()) << "Memory not correctly tracked, allocations done is not equal to the trackinglist.";


			auto t_It = m_MemoryTrackPolicy.m_TrackingList.find(a_Instance.ptr);
			ASSERT_NE(t_It, m_MemoryTrackPolicy.m_TrackingList.end()) << "Memory allocation doesn't exist on the tracking list.";

			ASSERT_EQ(a_Instance.size, t_It->second) << "Memory allocation doesn't share the size.";
		};

		void CheckAllocDoesntExists(const MemoryTrackInstance& a_Instance)
		{
			ASSERT_EQ(allocationsDone, m_MemoryTrackPolicy.m_TrackingList.size()) << "Memory not correctly tracked, allocations done is not equal to the trackinglist.";

			ASSERT_EQ(m_MemoryTrackPolicy.m_TrackingList.find(a_Instance.ptr), m_MemoryTrackPolicy.m_TrackingList.end()) << "Memory allocation exist on the tracking list while it shouldn't.";
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
		const size_t t_RandomAllocSize = Utils::RandomUintMinMax(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);

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