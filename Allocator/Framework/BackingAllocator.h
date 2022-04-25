#pragma once
#define WIN32_LEAN_AND_MEAN
#include <cstdint>

namespace BB
{
	struct BackingAllocator
	{
		BackingAllocator(); //Get needed info for the backing allocator.
	};

	static BackingAllocator virtualAllocBackingAllocator;


	/// <summary>
	/// Allocates virtual memory in pages, the amount allocated will be in multiples of BB::PAGESIZE;
	/// </summary>
	/// <param name="a_Start"> The previous pointer used to commit the backing memory, nullptr if this is the first instance of allocation. </param>
	/// <param name="a_Size"> size of the virtual memory allocation in bytes, will be internally changed to be a multiple of BB::PAGESIZE</param>
	/// <returns>Pointer to the start of the virtual memory. </returns>
	void* mallocVirtual(void* a_Start, size_t a_Size);
	void freeVirtual(void* a_Ptr);
}