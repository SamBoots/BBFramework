#pragma once
#define WIN32_LEAN_AND_MEAN
#include <cstdint>

namespace BB
{
	constexpr size_t INITIAL_BACKING_SIZE = 268435456; //256 MB
	constexpr size_t EXPANDING_BACKING_SIZE = 4194304; //4 MB

	constexpr size_t COMMIT_PER_SECTION = 4194304;
	
	struct BackingAllocator
	{
		BackingAllocator();

		void* Alloc(size_t a_Size);
		void Free(void* a_Ptr);

		struct FreePages
		{
			size_t reservedSize;
			FreePages* pNext;
		};

		struct PageHeader
		{
			size_t size;
		};

		FreePages* m_FreePages;
		uint8_t* m_Begin = nullptr;
		size_t m_AmountAllocated;

	private:
		uint8_t* ReserveBacking(const size_t a_Size);
	};

	static BackingAllocator virtualAllocBackingAllocator;
	static size_t PAGESIZE;
}