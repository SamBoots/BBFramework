#pragma once
#include <cstdint>

namespace BB
{
	struct BackingAllocator
	{
		BackingAllocator();

		void ReserveAndCommitSpace(size_t a_Size);
		void* Alloc(size_t a_Size);

		size_t backingSize;
		size_t backingCommited;
		size_t backingUsed;

		uint8_t* begin;
		uint8_t* current;
	};

	static BackingAllocator virtualAllocBackingAllocator;
	static size_t PAGESIZE;
}