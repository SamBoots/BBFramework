#include "pch.h"
#include "BackingAllocator.h"

#include <Windows.h>
#include <memoryapi.h>
#include <sysinfoapi.h>

using namespace BB;

BackingAllocator::BackingAllocator()
{
	SYSTEM_INFO t_SystemInfo;
	GetSystemInfo(&t_SystemInfo);
	PAGESIZE = t_SystemInfo.dwAllocationGranularity;
	
	//just reserve a lot lmao
	ReserveAndCommitSpace(PAGESIZE * 8192);
}

void BackingAllocator::ReserveAndCommitSpace(size_t a_Size)
{
	BB_ASSERT(!(a_Size % PAGESIZE), "Backing allocator's designated size is not a multiple of PAGESIZE");
	backingSize = a_Size;
	backingCommited = backingSize;

	begin = reinterpret_cast<uint8_t*>(
		VirtualAlloc(NULL, static_cast<DWORD>(backingSize), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	current = begin;

	std::cout << GetLastError() << std::endl;
}

void* BB::BackingAllocator::BackingAllocator::Alloc(size_t a_Size)
{
	BB_EXCEPTION(backingUsed + a_Size < backingCommited, "Not enough memory allocated in the BackingAllocator.");
	backingUsed += a_Size;
	uint8_t* returnedMemory = current;
	current += a_Size;
	return returnedMemory;
}
