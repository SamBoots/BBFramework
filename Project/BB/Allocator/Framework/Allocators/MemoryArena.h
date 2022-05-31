#pragma once

//Source on idea: https://blog.molecular-matters.com/2011/06/29/designing-extensible-modular-classes/

		//needs replacement by a custom hashmap.
#include <unordered_map>
#include "Utils/pointerUtils.h"

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
			static const size_t BOUNDRY_FRONT = 0;
			static const size_t BOUNDRY_BACK = 0;

			inline void AddBoundries(void*, const size_t) const {};
			inline void CheckBoundries(void*) const {}
		};

		constexpr const size_t BoundryCheckValue = 128;

		struct BoundsCheck
		{
			~BoundsCheck();

			static const size_t BOUNDRY_FRONT = sizeof(size_t);
			static const size_t BOUNDRY_BACK = sizeof(size_t);

			void AddBoundries(void* a_FrontPtr, size_t a_AllocSize);
			void CheckBoundries(void* a_FrontPtr);
			void Clear();

			//needs replacement by a custom hashmap.
			//First pointer = front.
			//Second pointer = back.
			std::unordered_map<void*, void*> m_BoundsList;
		};

		struct No_MemoryTrack
		{
			inline void OnAlloc(void*, size_t) const {}
			inline void OnDealloc(void*) const {}
			inline void Clear() const {}
		};

		struct Count_MemoryTrack
		{
			~Count_MemoryTrack();
			void OnAlloc(void* a_Ptr, size_t a_Size);
			void OnDealloc(void* a_Ptr);
			void Clear();

			//needs replacement by a custom hashmap.
			std::unordered_map<void*, size_t> m_TrackingList;
		};

		struct No_MemoryTagging
		{
			inline void TagAlloc(void*, size_t) const {}
			inline void TagDealloc(void*) const {}
		};
	}


	template <class Allocator, class ThreadPolicy, class BoundsCheckPolicy, class MemoryTrackPolicy>
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

			const size_t t_AllocSize = a_Size + BoundsCheckPolicy::BOUNDRY_FRONT + BoundsCheckPolicy::BOUNDRY_BACK;

			uint8_t* allocatedMemory = static_cast<uint8_t*>(m_Allocator.Alloc(t_AllocSize, a_Alignment));
			m_BoundsCheckPolicy.AddBoundries(allocatedMemory, t_AllocSize);

			//m_MemoryTaggingPolicy.TagAlloc(returnMemory, t_AllocSize);
			m_MemoryTrackPolicy.OnAlloc(allocatedMemory, t_AllocSize);

			m_ThreadPolicy.Leave();

			return pointerutils::Add(allocatedMemory, BoundsCheckPolicy::BOUNDRY_FRONT);
		}
		void Free(void* a_Ptr)
		{
			m_ThreadPolicy.Enter();

			void* originalMemory = pointerutils::Subtract(a_Ptr, BoundsCheckPolicy::BOUNDRY_FRONT);
			m_BoundsCheckPolicy.CheckBoundries(originalMemory);
			//m_MemoryTaggingPolicy.TagDealloc(originalMemory);
			m_MemoryTrackPolicy.OnDealloc(originalMemory);

			m_Allocator.Free(originalMemory);

			m_ThreadPolicy.Leave();
		}

		void Clear()
		{
			m_ThreadPolicy.Enter();

			m_BoundsCheckPolicy.Clear();
			m_MemoryTrackPolicy.Clear();
			m_Allocator.Clear();

			m_ThreadPolicy.Leave();
		}

	protected:
		Allocator m_Allocator;
		ThreadPolicy m_ThreadPolicy;
		BoundsCheckPolicy m_BoundsCheckPolicy;
		MemoryTrackPolicy m_MemoryTrackPolicy;
	};

	template <class MemoryArena, class MemoryTaggingPolicy>
	struct MemoryArenaProxy
	{
		MemoryArena m_MemoryArena;
		MemoryTaggingPolicy m_MemoryTaggingPolicy;
		
	};
}