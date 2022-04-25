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


	enum class MEM_VIRTUAL_CMD
	{
		RESERVE = 0, //WINDOWS VIRTUAL RESERVE
		COMMIT = 1, //WINDOWS VIRTUAL COMMIT
		RESERVE_COMMIT = 2 //LINUX MMAP
	};

	/// <summary>
	/// Allocates virtual memory in pages, the amount allocated will be in multiples of BB::PAGESIZE;
	/// </summary>
	/// <param name="a_Start"> where the pages will start, nullptr if you do not want not care about the start position. </param>
	/// <param name="a_Size"> size of the virtual memory allocation in bytes, will be internally changed to be a multiple of BB::PAGESIZE</param>
	/// <param name="a_Cmd"> the type of memory you want to reserve, read the respective virtual allocator API for the platform.</param>
	/// <returns>Pointer to the start of the virtual memory. </returns>
	void* mallocVirtual(void* a_Start, size_t a_Size, MEM_VIRTUAL_CMD a_Cmd) ;
	void freeVirtual(void* a_Ptr);
}