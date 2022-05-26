#pragma once
#include "pch.h"
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
			void* m_Start;
			void* m_Buffer;
			uintptr_t m_End;
		};

		struct FreelistAllocator
		{
			FreelistAllocator(const size_t a_Size);
			~FreelistAllocator();

			//just delete these for safety, copies might cause errors.
			FreelistAllocator(const FreelistAllocator&) = delete;
			FreelistAllocator(const FreelistAllocator&&) = delete;
			FreelistAllocator& operator =(const FreelistAllocator&) = delete;
			FreelistAllocator& operator =(FreelistAllocator&&) = delete;

			void* Alloc(size_t a_Size, size_t a_Alignment);
			void Free(void* a_Ptr);
			void Clear() const;

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
			uint8_t* m_Start = nullptr;
			FreeBlock* m_FreeBlocks;
			size_t m_TotalAllocSize;
		};

		//struct PoolAllocator
		//{
		//	PoolAllocator(const size_t a_ObjectSize, const size_t a_ObjectCount, const size_t a_Alignment);
		//	~PoolAllocator();

		//	//just delete these for safety, copies might cause errors.
		//	PoolAllocator(const PoolAllocator&) = delete;
		//	PoolAllocator(const PoolAllocator&&) = delete;
		//	PoolAllocator& operator =(const PoolAllocator&) = delete;
		//	PoolAllocator& operator =(PoolAllocator&&) = delete;

		//	void* Alloc(size_t a_Size, size_t);
		//	void Free(void* a_Ptr);
		//	void Clear();

		//	size_t m_Alignment;
		//	size_t m_ObjectCount;
		//	void** m_Start = nullptr;
		//	void** m_Pool;
		//};
	}

	template <typename T, typename MemoryArena>
	inline T* BBalloc(MemoryArena& a_Arena)
	{
		return new (reinterpret_cast<T*>(a_Arena.Alloc(sizeof(T), __alignof(T)))) T();
	}

	template <typename T, typename MemoryArena>
	inline T* BBalloc(MemoryArena& a_Arena, const T& a_T)
	{
		return new (reinterpret_cast<T*>(a_Arena.Alloc(sizeof(T), __alignof(T)))) T(a_T);
	}

	template <typename T, typename MemoryArena>
	inline T* BBallocArray(MemoryArena& a_Arena, size_t a_Length)
	{
		BB_ASSERT(a_Length != 0, "Trying to allocate an array with a length of 0.");

		uint8_t t_HeaderSize = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0) t_HeaderSize += 1;

		//Allocate the array, but shift it by sizeof(size_t) bytes forward to allow the size of the header to be put in as well.
		T* ptr = (reinterpret_cast<T*>(a_Arena.Alloc(sizeof(T) * (a_Length + t_HeaderSize), __alignof(T)))) + t_HeaderSize;

		//Store the size of the array inside the first element of the pointer.
		*(reinterpret_cast<size_t*>(ptr) - 1) = a_Length;

		//Create the elements.
		for (size_t i = 0; i < a_Length; i++)
			new (&ptr[i]) T;

		return ptr;
	}

	template <typename MemoryArena>
	inline void BBFree(MemoryArena& a_Arena, void* a_Ptr)
	{
		BB_ASSERT(a_Ptr != nullptr, "Trying to free a nullptr");
		a_Arena.Free(a_Ptr);
	}

	template <typename T, typename MemoryArena>
	inline void BBFreeArray(MemoryArena& a_Arena, T* a_Ptr)
	{
		BB_ASSERT(a_Ptr != nullptr, "Trying to freeArray a nullptr");

		//get the array size
		size_t t_Length = *(reinterpret_cast<size_t*>(a_Ptr) - 1);

		for (size_t i = 0; i < t_Length; i++)
			a_Ptr[i].~T();

		size_t t_HeaderSize = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0) t_HeaderSize += 1;

		a_Arena.Free(a_Ptr - t_HeaderSize);
	}
}

//inline void* operator new(size_t a_Bytes, BB::memory::LinearAllocator* a_Allocator)
//{
//	return a_Allocator->alloc(a_Bytes);
//};