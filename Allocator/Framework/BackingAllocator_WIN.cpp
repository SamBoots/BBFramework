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
	BB_WARNING(a_Size < PAGESIZE * 64, "Virtual Alloc is smaller then 4 MB, try to make allocators larger then 4 MB.");

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