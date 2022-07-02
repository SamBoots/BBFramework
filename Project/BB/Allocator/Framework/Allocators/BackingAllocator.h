#pragma once
#include <cstdint>

namespace BB
{
	
	enum class virtual_reserve_extra : size_t
	{
		none = 1, //Don't reserve more virtual space, use this if you will never resize a mallocVirtual address.
#ifdef _X64
		half = 64, //reserve 64 times more virtual space (16 times more on x86).
		standard = 128, //reserve 128 times more virtual space (32 times more on x86).
		extra = 256  //reserve 256 times more virtual space (64 times more on x86).
#endif //_X64
#ifdef _X86
		half = 16, //reserve 64 times more virtual space (16 times more on x86).
		standard = 32, //reserve 128 times more virtual space (32 times more on x86).
		extra = 64  //reserve 256 times more virtual space (64 times more on x86).
#endif //_X86
	};

	/// <summary>
	/// Allocates virtual memory.;
	/// </summary>
	/// <param name="a_Start"> The previous pointer used to commit the backing memory, nullptr if this is the first instance of allocation. </param>
	/// <param name="a_Size"> size of the virtual memory allocation in bytes, will be changed to be above OSDevice.virtual_memory_minimum_allocation and a multiple of OSDevice.virtual_memory_page_size.
	/// If a_Start is not a nullptr it will extend the commited range, will also be changed similiarly to normal.</param>
	/// <param name="a_ReserveSize"> How much extra memory is reserved for possible resizes. Default is virtual_reserve_extra::standard, which will reserve 128 times more virtual space (64 times more on x86).
	/// <returns>Pointer to the start of the virtual memory. </returns>
	void* mallocVirtual(void* a_Start, size_t& a_Size, const virtual_reserve_extra a_ReserveSize = virtual_reserve_extra::standard);
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
}