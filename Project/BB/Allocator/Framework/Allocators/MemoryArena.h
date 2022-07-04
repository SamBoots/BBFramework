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


	template <class Allocator, class ThreadPolicy>
	struct MemoryArena
	{
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
		Allocator m_Allocator;
		ThreadPolicy m_ThreadPolicy;

#ifdef _DEBUG
		MemoryDebugTools::BoundsCheck m_BoundsCheck;
		MemoryDebugTools::MemoryTrack m_MemoryTrack;
#endif //_DEBUG
	};
}