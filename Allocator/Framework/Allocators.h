#pragma once
#define WIN32_LEAN_AND_MEAN

#include <cstdint>

#define BB_NEW 

namespace BB
{

	namespace memory
	{


		struct LinearAllocator
		{
			LinearAllocator(const size_t a_Size);
			~LinearAllocator();

			//just delete these for safety, copies might cause errors.
			LinearAllocator(const LinearAllocator&) = delete;
			LinearAllocator(const LinearAllocator&&) = delete;
			LinearAllocator& operator =(const LinearAllocator&) = delete;
			LinearAllocator& operator =(LinearAllocator&&) = delete;

			void* Alloc(size_t a_Size, uint8_t a_Alignment);
			void clear();

		private:
			uint8_t* m_Buffer;
			uint8_t* m_Start;

			const size_t m_Size;
		};




	}

	template <typename T>
	inline T* AllocNew(memory::LinearAllocator& a_Allocator)
	{
		return new (reinterpret_cast<T*>(a_Allocator.Alloc(sizeof(T), __alignof(T)))) T();
	}

	template <typename T>
	inline T* AllocNew(memory::LinearAllocator& a_Allocator, const T& a_T)
	{
		return new (reinterpret_cast<T*>(a_Allocator.Alloc(sizeof(T), __alignof(T)))) T(a_T);
	}

	template <typename T>
	inline T* AllocNewArray(memory::LinearAllocator& a_Allocator, size_t a_Length)
	{
		uint8_t t_HeaderSize = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0) t_HeaderSize += 1;

		//Allocate the array, but shift it 8 bytes forward to allow the size of the header to be put in as well.
		T* ptr = (reinterpret_cast<T*>(a_Allocator.Alloc(sizeof(T) * (a_Length * t_HeaderSize), __alignof(T)))) + t_HeaderSize;

		//Store the size of the array inside the first element of the pointer.
		*(reinterpret_cast<size_t*>(ptr) - 1) = a_Length;

		//Create the elements.
		for (size_t i = 0; i < a_Length; i++)
			new (&ptr) T;

		return ptr;
	}


}

//inline void* operator new(size_t a_Bytes, BB::memory::LinearAllocator* a_Allocator)
//{
//	return a_Allocator->alloc(a_Bytes);
//};