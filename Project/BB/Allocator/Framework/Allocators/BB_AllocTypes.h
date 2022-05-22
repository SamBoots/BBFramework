#pragma once

#include "MemoryArena.h"
#include "Allocators.h"

namespace BB
{
	constexpr const size_t kbSize = 1024;
	constexpr const size_t mbSize = kbSize * 1024;
	constexpr const size_t gbSize = mbSize * 1024;


	//Default types
	typedef BB::MemoryArena<BB::allocators::LinearAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::Count_MemoryTrack> LinearAllocator_t;
	typedef BB::MemoryArena<BB::allocators::FreelistAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::Count_MemoryTrack> FreeListAllocator_t;
	typedef BB::MemoryArena<BB::allocators::PoolAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::Count_MemoryTrack> PoolAllocator_t;

	//specialized types.
	typedef BB::MemoryArena<BB::allocators::LinearAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTrack> unsafeLinearAllocator_t;
	typedef BB::MemoryArena<BB::allocators::FreelistAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTrack> unsafeFreeListAllocator_t;
	typedef BB::MemoryArena<BB::allocators::PoolAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTrack> unsafePoolAllocator_t;
}