#pragma once

#include "MemoryArena.h"
#include "Allocators.h"

namespace BB
{
	constexpr const size_t kbSize = 1024;
	constexpr const size_t mbSize = kbSize * 1024;
	constexpr const size_t gbSize = mbSize * 1024;

#ifdef _DEBUG
	//Default types
	using LinearAllocator_t = BB::MemoryArena<BB::allocators::LinearAllocator, BB::ThreadPolicy::Single_Thread, true>;
	using FreeListAllocator_t = BB::MemoryArena<BB::allocators::FreelistAllocator, BB::ThreadPolicy::Single_Thread, true>;
	using POW_FreeListAllocator_t = BB::MemoryArena<BB::allocators::POW_FreelistAllocator, BB::ThreadPolicy::Single_Thread, true>;
#else
	//Default types
	using LinearAllocator_t = BB::MemoryArena<BB::allocators::LinearAllocator, BB::ThreadPolicy::Single_Thread, false>;
	using FreeListAllocator_t = BB::MemoryArena<BB::allocators::FreelistAllocator, BB::ThreadPolicy::Single_Thread, false>;
	using POW_FreeListAllocator_t = BB::MemoryArena<BB::allocators::POW_FreelistAllocator, BB::ThreadPolicy::Single_Thread, false>;
#endif

#pragma region AllocationFunctions
	inline void* BBalloc(Allocator a_Arena, const size_t a_Size)
	{
		return a_Arena.func(a_Arena.allocator, a_Size, 1, nullptr);
	}

	template <typename T>
	inline T* BBalloc(Allocator a_Arena, const T& a_T)
	{
		return new (reinterpret_cast<T*>(a_Arena.func(a_Arena.allocator, sizeof(T), __alignof(T), nullptr))) T(a_T);
	}

	template <typename T, typename... Args>
	inline T* BBalloc(Allocator a_Arena, Args&&... a_Args)
	{
		return new (reinterpret_cast<T*>(a_Arena.func(a_Arena.allocator, sizeof(T), __alignof(T), nullptr))) T(std::forward<Args>(a_Args)...);
	}

	template <typename T>
	inline T* BBallocArray(Allocator a_Arena, size_t a_Length)
	{
		BB_ASSERT(a_Length != 0, "Trying to allocate an array with a length of 0.");

		size_t t_HeaderSize;

		if constexpr (sizeof(size_t) % sizeof(T) > 0)
			t_HeaderSize = sizeof(size_t) / sizeof(T) + 1;
		else
			t_HeaderSize = sizeof(size_t) / sizeof(T);

		//Allocate the array, but shift it by sizeof(size_t) bytes forward to allow the size of the header to be put in as well.
		T* ptr = (reinterpret_cast<T*>(a_Arena.func(a_Arena.allocator, sizeof(T) * (a_Length + t_HeaderSize), __alignof(T), nullptr))) + t_HeaderSize;

		//Store the size of the array inside the first element of the pointer.
		*(reinterpret_cast<size_t*>(ptr) - 1) = a_Length;

		//Create the elements.
		for (size_t i = 0; i < a_Length; i++)
			new (&ptr[i]) T();

		return ptr;
	}

	inline void BBfree(Allocator a_Arena, void* a_Ptr)
	{
		BB_ASSERT(a_Ptr != nullptr, "Trying to free a nullptr");
		a_Arena.func(a_Arena.allocator, 0, 0, a_Ptr);
	}

	template <typename T>
	inline void BBfreeArray(Allocator a_Arena, T* a_Ptr)
	{
		BB_ASSERT(a_Ptr != nullptr, "Trying to freeArray a nullptr");
		size_t t_HeaderSize;
		if constexpr (sizeof(size_t) % sizeof(T) > 0)
			t_HeaderSize = sizeof(size_t) / sizeof(T) + 1;
		else
			t_HeaderSize = sizeof(size_t) / sizeof(T);

		a_Arena.func(a_Arena.allocator, 0, 0, a_Ptr - t_HeaderSize);
	}

	template <typename T>
	inline void BBdestroy(Allocator a_Arena, T* a_Ptr)
	{
		BB_ASSERT(a_Ptr != nullptr, "Trying to free a nullptr");
		a_Ptr->~T();
		a_Arena.func(a_Arena.allocator, 0, 0, a_Ptr);
	}

	template <typename T>
	inline void BBdestroyArray(Allocator a_Arena, T* a_Ptr)
	{
		BB_ASSERT(a_Ptr != nullptr, "Trying to freeArray a nullptr");

		//get the array size
		size_t t_Length = *(reinterpret_cast<size_t*>(a_Ptr) - 1);

		for (size_t i = 0; i < t_Length; i++)
			a_Ptr[i].~T();

		size_t t_HeaderSize;
		if constexpr (sizeof(size_t) % sizeof(T) > 0)
			t_HeaderSize = sizeof(size_t) / sizeof(T) + 1;
		else
			t_HeaderSize = sizeof(size_t) / sizeof(T);

		a_Arena.func(a_Arena.allocator, 0, 0, a_Ptr - t_HeaderSize);
	}
#pragma endregion // AllocationFunctions

#pragma region Debug
	//1GB of debug size, more can be reserved automatically.
	constexpr const size_t debugSize = gbSize * 1;
	//A global debug allocator.
	static FreeListAllocator_t DebugAllocator(debugSize);

	//Global alloc that uses a debug allocator. Only use this when debugging or testing features. It will pop warnings in release mode.
	template <typename T, typename... Args>
	T* BBglobalalloc(Args&&... a_Args)
	{
#ifndef _DEBUG
		BB_WARNING(false, "BBglobalalloc used while in release mode. This should only happen if you are testing unfinished features. Consinder using a temporary or system allocator instead.", WarningType::OPTIMALIZATION);
#endif //_DEBUG
		return BBalloc<T, Args...>(DebugAllocator, a_Args...);
	}

	//Global alloc that uses a debug allocator. Only use this when debugging or testing features. It will pop warnings in release mode.
	template <typename T, typename... Args>
	T* BBglobalallocarray(size_t a_Count)
	{
#ifndef _DEBUG
		BB_WARNING(false, "BBglobalalloc used while in release mode. This should only happen if you are testing unfinished features. Consinder using a temporary or system allocator instead.", WarningType::OPTIMALIZATION);
#endif //_DEBUG
		return BBallocArray<T>(DebugAllocator, a_Count);
	}

	//Global destroy that uses a debug allocator. Only use this when debugging or testing features. It will pop warnings in release mode.
	template <typename T>
	void BBglobaldestroy(T* a_Ptr)
	{
		BBdestroy<T>(DebugAllocator, a_Ptr);
	}

	//Global destroy that uses a debug allocator. Only use this when debugging or testing features. It will pop warnings in release mode.
	template <typename T>
	void BBglobaldestroyarray(T* a_Ptr)
	{
		BBdestroyArray<T>(DebugAllocator, a_Ptr);
	}

#pragma endregion // Debug
}