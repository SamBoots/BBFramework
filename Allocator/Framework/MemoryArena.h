#pragma once

//Source on idea: https://blog.molecular-matters.com/2011/06/29/designing-extensible-modular-classes/
#include "Allocators.h"

namespace BB
{
	namespace memorypolicies
	{
		struct Single_Thread
		{
			inline void Enter(void) const {};
			inline void Leave(void) const {};
		};

		struct No_BoundsCheck
		{
			No_BoundsCheck(size_t) {};
			const size_t memorySize = 0;

			inline void CheckFront(const void*) const {}
			inline void CheckBack(const void*) const {}
		};

		struct No_MemoryTrack
		{
			inline void OnAlloc(void*, size_t, size_t) const {}
			inline void OnDealloc(void*) const {}
		};

		struct No_MemoryTagging
		{
			inline void TagAlloc(void*, size_t) const {}
			inline void TagDealloc(void*) const {}
		};
	}


	template <class Allocator, class ThreadPolicy, class BoundsCheckPolicy, class MemoryTaggingPolicy>
	struct MemoryArena
	{
		MemoryArena(const size_t a_Size)
			: m_Allocator(a_Size), m_BoundsCheckPolicy(a_Size)
		{}

		void* Alloc(size_t a_Size, size_t a_Alignment)
		{
			m_ThreadPolicy.Enter();

			const size_t t_AllocSize = a_Size;

			uint8_t* returnMemory = static_cast<uint8_t*>(m_Allocator.Alloc(a_Size, a_Alignment));

			m_MemoryTaggingPolicy.TagAlloc(returnMemory, t_AllocSize);

			m_ThreadPolicy.Leave();

			return returnMemory;
		}
		void Free(void* a_Ptr)
		{
			m_ThreadPolicy.Enter();

			uint8_t* originalMemory = static_cast<uint8_t*>(a_Ptr);

			m_MemoryTaggingPolicy.TagDealloc(originalMemory);

			m_Allocator.Free(originalMemory);

			m_ThreadPolicy.Leave();
		}

		void Clear()
		{
			m_ThreadPolicy.Enter();

			m_Allocator.Clear();

			m_ThreadPolicy.Leave();
		}

	private:
		Allocator m_Allocator;
		ThreadPolicy m_ThreadPolicy;
		BoundsCheckPolicy m_BoundsCheckPolicy;
		MemoryTaggingPolicy m_MemoryTaggingPolicy;
	};

	template <class MemoryArena, class MemoryTrackPolicy>
	struct MemoryArenaProxy
	{
		MemoryArena m_MemoryArena;

		MemoryTrackPolicy m_MemoryTrackPolicy;
	};
}