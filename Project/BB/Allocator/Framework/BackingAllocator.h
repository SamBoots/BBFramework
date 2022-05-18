#pragma once
#include <cstdint>

namespace BB
{
	/// <summary>
	/// Allocates virtual memory in pages, the amount allocated will be in multiples of BB::PAGESIZE;
	/// </summary>
	/// <param name="a_Start"> The previous pointer used to commit the backing memory, nullptr if this is the first instance of allocation. </param>
	/// <param name="a_Size"> size of the virtual memory allocation in bytes, will be internally changed to be a multiple of BB::PAGESIZE</param>
	/// <returns>Pointer to the start of the virtual memory. </returns>
	void* mallocVirtual(void* a_Start, size_t a_Size);
	void freeVirtual(void* a_Ptr);

	struct PagePool
	{
		PagePool();
		~PagePool();

		void* AllocHeader()
		{
			void* t_Header = pool;
			pool = reinterpret_cast<void**>(*pool);
			return t_Header;
		};
		void FreeHeader(void* a_Ptr)
		{
			(*reinterpret_cast<void**>(a_Ptr)) = pool;
			pool = reinterpret_cast<void**>(a_Ptr);
		};

		void* bufferStart;
		void** pool;
	};

	static PagePool pagePool{};


}