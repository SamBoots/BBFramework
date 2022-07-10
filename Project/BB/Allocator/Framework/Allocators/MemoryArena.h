#pragma once

//Source on idea: https://blog.molecular-matters.com/2011/06/29/designing-extensible-modular-classes/

		//needs replacement by a custom hashmap.
#include <unordered_map>
#include "Utils/PointerUtils.h"

namespace BB
{
#ifdef _DEBUG
	namespace MemoryDebugTools
	{
		constexpr const uintptr_t BoundryCheckValue = 0xDEADBEEFDEADBEEF;
		constexpr const size_t BOUNDRY_FRONT = sizeof(size_t);
		constexpr const size_t BOUNDRY_BACK = sizeof(size_t);

		struct BoundsCheck
		{
			~BoundsCheck();

			void AddBoundries(void* a_FrontPtr, size_t a_AllocSize);
			void CheckBoundries(void* a_FrontPtr);
			void Clear();

			//needs replacement by a custom hashmap.
			//First pointer = front.
			//Second pointer = back.
			std::unordered_map<void*, void*> m_BoundsList;
		};

		struct MemoryTrack
		{
			~MemoryTrack();
			void OnAlloc(void* a_Ptr, size_t a_Size);
			void OnDealloc(void* a_Ptr);
			void Clear();

			//needs replacement by a custom hashmap.
			std::unordered_map<void*, size_t> m_TrackingList;
		};
	}
#endif //_DEBUG

	namespace ThreadPolicy
	{
		struct Single_Thread
		{
			inline void Enter(void) const {};
			inline void Leave(void) const {};
		};
	}

	typedef void* (*AllocateFunc)(void* a_AllocatorData,  size_t a_Size, size_t a_Alignment, void* a_OldPtr);
	struct Allocator
	{
		AllocateFunc func;
		void* allocator;
	};

	template<typename T>
	void* Realloc(void* a_Allocator, size_t a_Size, size_t a_Alignment, void* a_Ptr)
	{
		if (a_Size > 0)
		{
			return reinterpret_cast<T*>(a_Allocator)->Alloc(a_Size, a_Alignment);
		}
		else
		{
			reinterpret_cast<T*>(a_Allocator)->Free(a_Ptr);
			return nullptr;
		}
	}

	template <class Allocator_Type, class ThreadPolicy>
	struct MemoryArena
	{
		operator Allocator()
		{
			Allocator t_AllocatorInterface;
			t_AllocatorInterface.allocator = this;
			t_AllocatorInterface.func = Realloc<MemoryArena<Allocator_Type, ThreadPolicy>>;
			return t_AllocatorInterface;
		}

		MemoryArena(const size_t a_Size)
			: m_Allocator(a_Size)
		{}
		
		MemoryArena(const size_t a_ObjectSize, const size_t a_ObjectCount, const size_t a_Alignment)
			: m_Allocator(a_ObjectSize, a_ObjectCount, a_Alignment)
		{}

		void* Alloc(size_t a_Size, size_t a_Alignment)
		{
			m_ThreadPolicy.Enter();

#ifdef _DEBUG
			//Add more room for the boundry checking.
			a_Size += MemoryDebugTools::BOUNDRY_FRONT + MemoryDebugTools::BOUNDRY_BACK;
#endif //_DEBUG
			void* allocatedMemory = m_Allocator.Alloc(a_Size, a_Alignment);
#ifdef _DEBUG
			//Do all the debugging tools.
			m_BoundsCheck.AddBoundries(allocatedMemory, a_Size);
			m_MemoryTrack.OnAlloc(allocatedMemory, a_Size);
			allocatedMemory = pointerutils::Add(allocatedMemory, MemoryDebugTools::BOUNDRY_FRONT);
#endif //_DEBUG
			m_ThreadPolicy.Leave();

			return allocatedMemory;
		}
		void Free(void* a_Ptr)
		{
			m_ThreadPolicy.Enter();
#ifdef _DEBUG
			//Adjust the pointer to the boundry that was being set.
			a_Ptr = pointerutils::Subtract(a_Ptr, MemoryDebugTools::BOUNDRY_FRONT);
			m_BoundsCheck.CheckBoundries(a_Ptr);
			m_MemoryTrack.OnDealloc(a_Ptr);
#endif //_DEBUG
			m_Allocator.Free(a_Ptr);

			m_ThreadPolicy.Leave();
		}

		void Clear()
		{
			m_ThreadPolicy.Enter();
#ifdef _DEBUG
			m_BoundsCheck.Clear();
			m_MemoryTrack.Clear();
#endif //_DEBUG
			m_Allocator.Clear();

			m_ThreadPolicy.Leave();
		}

	protected:
		Allocator_Type m_Allocator;
		ThreadPolicy m_ThreadPolicy;

#ifdef _DEBUG
		MemoryDebugTools::BoundsCheck m_BoundsCheck;
		MemoryDebugTools::MemoryTrack m_MemoryTrack;
#endif //_DEBUG
	};


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
}