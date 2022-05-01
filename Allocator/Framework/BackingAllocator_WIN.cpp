#include "pch.h"
#include "BackingAllocator.h"
#include "pointerUtils.h"

#include <Windows.h>
#include <memoryapi.h>
#include <sysinfoapi.h>

#include <cmath>


using namespace BB;

constexpr const size_t RESERVEMULTIPLICATION = 16;
static size_t PAGESIZE;

struct PageHeader
{
	size_t bytesCommited;
	size_t bytesUsed;
	size_t bytesReserved;
};

BackingAllocator::BackingAllocator()
{
	SYSTEM_INFO t_SystemInfo;
	GetSystemInfo(&t_SystemInfo);
	PAGESIZE = t_SystemInfo.dwAllocationGranularity;
}


static size_t RoundUp(size_t a_NumToRound, size_t a_Multiple)
{
	BB_ASSERT(a_Multiple, "Multiple is 0!");
	return ((a_NumToRound + a_Multiple - 1) / a_Multiple) * a_Multiple;
}

void* BB::mallocVirtual(void* a_Start, size_t a_Size)
{
	BB_WARNING(a_Size > PAGESIZE * 64, "Virtual Alloc is smaller then 4 MB, try to make allocators larger then 4 MB.");

	size_t t_AdjustedSize = RoundUp(a_Size, PAGESIZE);
	//Check the pageHeader
	if (a_Start != nullptr)
	{
		PageHeader* t_Header = reinterpret_cast<PageHeader*>(pointerutils::Subtract(a_Start, sizeof(PageHeader)));
		void* t_ReturnAddress = pointerutils::Add(a_Start, t_Header->bytesUsed);
		//If the amount commited is enough just move the pointer and return it.
		if ((t_Header->bytesCommited - t_Header->bytesUsed) > t_AdjustedSize)
		{
			t_Header->bytesUsed += a_Size;
			return t_ReturnAddress;
		}
		
		void* t_Address = pointerutils::Add(a_Start, t_Header->bytesCommited);

		//If the amount commited is not enough check if there is enough reserved, if not reserve more.
		if (t_Header->bytesReserved < t_AdjustedSize)
		{
			size_t t_AdditionalReserve = t_AdjustedSize * RESERVEMULTIPLICATION;
			VirtualAlloc(t_Address, t_AdditionalReserve, MEM_RESERVE, PAGE_READWRITE);
			t_Header->bytesReserved += t_AdditionalReserve;
		}
		t_Header->bytesCommited += t_AdjustedSize;
		t_Header->bytesUsed += a_Size;
		t_Header->bytesReserved -= t_AdjustedSize;
		VirtualAlloc(t_Address, t_AdjustedSize, MEM_COMMIT, PAGE_READWRITE);

		return t_ReturnAddress;
	}

	size_t t_AdditionalReserve = t_AdjustedSize * RESERVEMULTIPLICATION;
	void* a_Address = VirtualAlloc(a_Start, t_AdditionalReserve, MEM_RESERVE, PAGE_READWRITE);
	VirtualAlloc(a_Address, t_AdjustedSize, MEM_COMMIT, PAGE_READWRITE);

	//Pageheader for new allocation.
	PageHeader* t_Header = reinterpret_cast<PageHeader*>(a_Address);
	t_Header->bytesCommited = t_AdjustedSize;
	t_Header->bytesUsed = a_Size + sizeof(PageHeader);
	t_Header->bytesReserved = t_AdditionalReserve - t_AdjustedSize;
	void* a_AdjustedAddress = pointerutils::Add(a_Address, sizeof(PageHeader));

	return a_AdjustedAddress;
}

void BB::freeVirtual(void* a_Ptr)
{
	VirtualFree(a_Ptr, 0, MEM_RELEASE);
}


#include <gtest/gtest.h>
#include "Utils/Math.h"

#pragma region Unit Test

struct MockAllocator
{
	MockAllocator(size_t a_Size)
	{
		maxSize = a_Size;
		start = reinterpret_cast<uint8_t*>(mallocVirtual(nullptr, a_Size));
		buffer = start;
	}

	void* Alloc(size_t a_Size)
	{
		maxSize -= a_Size;
		currentSize += a_Size;
		buffer += a_Size;
		return buffer;
	};
	//Not supporting free yet.
	//void free(size_t a_Size);

	size_t maxSize;
	size_t currentSize = 0;
	uint8_t* start;
	uint8_t* buffer;
};

TEST(MemoryAllocators_Backend_Windows, COMMIT_RESERVE_PAGES)
{
	//Allocator size is equal to half a page, it will allocate an entire page in the background anyway.
	MockAllocator t_Allocator(PAGESIZE / 2);
	ASSERT_EQ(GetLastError(), 0x0) << "Windows API error on creating the MockAllocator.";

	PageHeader lastHeader = *reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));

	ASSERT_EQ(lastHeader.bytesUsed, t_Allocator.maxSize + sizeof(PageHeader)) << "Used amount is wrong.";
	ASSERT_EQ(lastHeader.bytesCommited, PAGESIZE) << "Commited amount is wrong.";
	ASSERT_EQ(lastHeader.bytesReserved, PAGESIZE * RESERVEMULTIPLICATION - lastHeader.bytesCommited) << "Reserved amount is wrong.";

	//Allocate memory equal to an entire page, it should NOT reserve/commit more memory.
	t_Allocator.Alloc(PAGESIZE - sizeof(PageHeader));
}

#pragma endregion