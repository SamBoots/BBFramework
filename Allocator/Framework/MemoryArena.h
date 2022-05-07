#pragma once

//Source on idea: https://blog.molecular-matters.com/2011/06/29/designing-extensible-modular-classes/

		//needs replacement by a custom hashmap.
#include <unordered_map>

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
			inline void OnAlloc(void*, size_t) const {}
			inline void OnDealloc(void*) const {}
		};

		struct Count_MemoryTrack
		{
			~Count_MemoryTrack()
			{
				for (auto& t_It : m_TrackingList)
				{
					std::cout << "Address: " << t_It.first << " Leak size: " << t_It.second << "\n";
				}
				BB_EXCEPTION(m_TrackingList.size() == 0, "Memory tracker reports a memory leak, Log of leaks have been posted.");
			}
			inline void OnAlloc(void* a_Ptr, size_t a_Size)
			{
				m_TrackingList.emplace(a_Ptr, a_Size);
			}
			inline void OnDealloc(void* a_Ptr)
			{
				m_TrackingList.erase(a_Ptr);
			}

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
			: m_Allocator(a_Size), m_BoundsCheckPolicy(a_Size)
		{}
		
		MemoryArena(const size_t a_ObjectSize, const size_t a_ObjectCount, const size_t a_Alignment)
			: m_Allocator(a_ObjectSize, a_ObjectCount, a_Alignment), m_BoundsCheckPolicy(a_ObjectSize * a_ObjectCount)
		{}

		void* Alloc(size_t a_Size, size_t a_Alignment)
		{
			m_ThreadPolicy.Enter();

			const size_t t_AllocSize = a_Size;

			uint8_t* returnMemory = static_cast<uint8_t*>(m_Allocator.Alloc(a_Size, a_Alignment));

			//m_MemoryTaggingPolicy.TagAlloc(returnMemory, t_AllocSize);
			m_MemoryTrackPolicy.OnAlloc(returnMemory, t_AllocSize);

			m_ThreadPolicy.Leave();

			return returnMemory;
		}
		void Free(void* a_Ptr)
		{
			m_ThreadPolicy.Enter();

			uint8_t* originalMemory = static_cast<uint8_t*>(a_Ptr);

			//m_MemoryTaggingPolicy.TagDealloc(originalMemory);
			m_MemoryTrackPolicy.OnDealloc(originalMemory);

			m_Allocator.Free(originalMemory);

			m_ThreadPolicy.Leave();
		}

		void Clear()
		{
			m_ThreadPolicy.Enter();

			m_Allocator.Clear();

			m_ThreadPolicy.Leave();
		}

		void* begin() const
		{
			return m_Allocator.begin();
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