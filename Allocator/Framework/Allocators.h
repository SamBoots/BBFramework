#pragma once
#define WIN32_LEAN_AND_MEAN

#include <cstdint>

namespace BB
{
	namespace allocators
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

			void* Alloc(size_t a_Size, size_t a_Alignment);
			void Free(void*);
			void Clear();

		private:
			uint8_t* m_Start;
			uint8_t* m_Buffer;
		};

		struct FreelistAllocator
		{
			FreelistAllocator(const size_t a_Size);
			~FreelistAllocator();

			//just delete these for safety, copies might cause errors.
			FreelistAllocator(const LinearAllocator&) = delete;
			FreelistAllocator(const LinearAllocator&&) = delete;
			FreelistAllocator& operator =(const LinearAllocator&) = delete;
			FreelistAllocator& operator =(LinearAllocator&&) = delete;

			void* Alloc(size_t a_Size, size_t a_Alignment);
			void Free(void* a_Ptr);

			struct AllocHeader 
			{
				size_t size;
				size_t adjustment;
			};
			struct FreeBlock
			{
				size_t size;
				FreeBlock* next;
			};
			uint8_t* m_Start;
			FreeBlock* m_FreeBlocks;
		};
	}

	template <typename T>
	inline T* AllocNew(allocators::LinearAllocator& a_Allocator)
	{
		return new (reinterpret_cast<T*>(a_Allocator.Alloc(sizeof(T), __alignof(T)))) T();
	}

	template <typename T>
	inline T* AllocNew(allocators::LinearAllocator& a_Allocator, const T& a_T)
	{
		return new (reinterpret_cast<T*>(a_Allocator.Alloc(sizeof(T), __alignof(T)))) T(a_T);
	}

	template <typename T>
	inline T* AllocNewArray(allocators::LinearAllocator& a_Allocator, size_t a_Length)
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

	template <typename T, typename MemoryArena>
	inline T* AllocNew(MemoryArena& a_Arena)
	{
		return new (reinterpret_cast<T*>(a_Arena.Alloc(sizeof(T), __alignof(T)))) T();
	}

	template <typename T, typename MemoryArena>
	inline T* AllocNew(MemoryArena& a_Arena, const T& a_T)
	{
		return new (reinterpret_cast<T*>(a_Arena.Alloc(sizeof(T), __alignof(T)))) T(a_T);
	}

	template <typename T, typename MemoryArena>
	inline T* AllocNewArray(MemoryArena& a_Arena, size_t a_Length)
	{
		uint8_t t_HeaderSize = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0) t_HeaderSize += 1;

		//Allocate the array, but shift it 8 bytes forward to allow the size of the header to be put in as well.
		T* ptr = (reinterpret_cast<T*>(a_Arena.Alloc(sizeof(T) * (a_Length * t_HeaderSize), __alignof(T)))) + t_HeaderSize;

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