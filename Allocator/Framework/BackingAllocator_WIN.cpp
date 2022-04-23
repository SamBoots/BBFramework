#include "pch.h"
#include "BackingAllocator.h"
#include "pointerUtils.h"

#include <Windows.h>
#include <memoryapi.h>
#include <sysinfoapi.h>

using namespace BB;

static int RoundUp(int a_NumToRound, int a_Multiple)
{
	BB_ASSERT(a_Multiple, "Multiple is 0!");
	return ((a_NumToRound + a_Multiple - 1) / a_Multiple) * a_Multiple;
}

BackingAllocator::BackingAllocator()
{
	SYSTEM_INFO t_SystemInfo;
	GetSystemInfo(&t_SystemInfo);
	PAGESIZE = t_SystemInfo.dwAllocationGranularity;
	
	//just reserve a lot lmao
	
	m_Begin = ReserveBacking(INITIAL_BACKING_SIZE);
	m_AmountAllocated = INITIAL_BACKING_SIZE;

	m_FreePages = reinterpret_cast<FreePages*>(VirtualAlloc(m_Begin, sizeof(FreePages), MEM_COMMIT, PAGE_READWRITE));
	m_FreePages->reservedSize += m_AmountAllocated;
	m_FreePages->pNext = nullptr;
}

void* BB::BackingAllocator::Alloc(size_t a_Size)
{
	BB_WARNING((a_Size < PAGESIZE * 64), "Allocator is smaller then 4 MB, try to make allocators larger then 4 MB.");
	FreePages* t_PreviousPages = nullptr;
	FreePages* t_CurrentPages = m_FreePages;

	size_t t_AllocSize = a_Size + sizeof(PageHeader);
	size_t t_MultipliedSize = RoundUp(a_Size, PAGESIZE);

	while (t_CurrentPages != nullptr)
	{
		if (t_CurrentPages->reservedSize > t_MultipliedSize)
		{
			uintptr_t t_Address = reinterpret_cast<uintptr_t>(VirtualAlloc(t_CurrentPages, t_MultipliedSize, MEM_COMMIT, PAGE_READWRITE));
			BB_EXCEPTION(t_Address, "Cannot commit pages for the block.");
			PageHeader* t_Header = reinterpret_cast<PageHeader*>(t_Address);
			t_Header->size = t_MultipliedSize;
			t_Address += sizeof(PageHeader);
			m_AmountAllocated += t_MultipliedSize;

			//create new block.
			{
				FreePages* t_NextPages = reinterpret_cast<FreePages*>(pointerutils::Add(t_CurrentPages, t_MultipliedSize));
				t_NextPages = reinterpret_cast<FreePages*>(VirtualAlloc(t_NextPages, sizeof(FreePages), MEM_COMMIT, PAGE_READWRITE));
				BB_EXCEPTION(t_NextPages, "Cannot commit a page for the next block.");
				t_NextPages->reservedSize = t_CurrentPages->reservedSize - t_MultipliedSize;
				t_NextPages->pNext = t_CurrentPages->pNext;

				if (t_PreviousPages != nullptr)
					t_PreviousPages->pNext = t_NextPages;
				else
					m_FreePages = t_NextPages;
			}

			return reinterpret_cast<void*>(t_Address);
		}
		t_PreviousPages = t_CurrentPages;
		t_CurrentPages = t_CurrentPages->pNext;
	}

	BB_WARNING(false, "Not enough backing memory reserved, reserving more pages as backup.");
	
	size_t t_ExpandSize = RoundUp(t_AllocSize, EXPANDING_BACKING_SIZE);
	ReserveBacking(t_ExpandSize);
	t_PreviousPages->reservedSize += t_ExpandSize;

	return Alloc(a_Size);
}

void BB::BackingAllocator::Free(void* a_Ptr)
{
	BB_ASSERT(a_Ptr != nullptr, "Nullptr send to BackingAllocator::Free!.");
	PageHeader* t_Header = reinterpret_cast<PageHeader*>(pointerutils::Subtract(a_Ptr, sizeof(PageHeader)));
	size_t t_BlockSize = t_Header->size;
	uintptr_t t_BlockStart = reinterpret_cast<uintptr_t>(a_Ptr);
	uintptr_t t_BlockEnd = t_BlockStart + t_BlockSize;

	FreePages* t_PreviousPages = nullptr;
	FreePages* t_CurrentPages = m_FreePages;

	while (t_CurrentPages != nullptr)
	{
		if (reinterpret_cast<uintptr_t>(t_CurrentPages) >= t_BlockEnd) break;
		t_PreviousPages = m_FreePages;
		m_FreePages = m_FreePages->pNext;
	}

	if (t_PreviousPages == nullptr)
	{
		t_PreviousPages = reinterpret_cast<FreePages*>(t_BlockStart);
		t_PreviousPages->reservedSize = t_Header->size;
		t_PreviousPages->pNext = t_CurrentPages;
		m_FreePages = t_PreviousPages;
	}
	else if (reinterpret_cast<uintptr_t>(t_PreviousPages) + t_PreviousPages->reservedSize == t_BlockStart)
	{
		t_PreviousPages->reservedSize += t_BlockSize;
	}
	else
	{
		FreePages* t_Temp = reinterpret_cast<FreePages*>(t_BlockStart);
		t_Temp->reservedSize = t_BlockSize;
		t_Temp->pNext = t_PreviousPages->pNext;
		t_PreviousPages->pNext = t_Temp;
		t_PreviousPages = t_Temp;
	}

	if (t_CurrentPages != nullptr && reinterpret_cast<uintptr_t>(t_CurrentPages) == t_BlockEnd)
	{
		t_PreviousPages->reservedSize = t_CurrentPages->reservedSize;
		t_PreviousPages->pNext = t_CurrentPages->pNext;
	}
}

uint8_t* BB::BackingAllocator::ReserveBacking(const size_t a_Size)
{
	BB_ASSERT(!(a_Size % PAGESIZE), "Backing allocator's reserve size is not a multiple of PAGESIZE");
	void* t_BeginAddress = pointerutils::Add(m_Begin, m_AmountAllocated);

	uint8_t* t_ReturnAddress = reinterpret_cast<uint8_t*>(VirtualAlloc(t_BeginAddress, a_Size, MEM_RESERVE, PAGE_READWRITE));
	m_AmountAllocated += a_Size;
	return t_ReturnAddress;
}