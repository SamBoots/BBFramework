#pragma once

#include "MemoryArena.h"
#include "Allocators.h"

namespace BB
{
	typedef BB::MemoryArena<BB::allocators::LinearAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTagging> unsafeLinearAllocator_t;
	typedef BB::MemoryArena<BB::allocators::FreelistAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTagging> unsafeFreeListAllocator_t;
	typedef BB::MemoryArena<BB::allocators::PoolAllocator, BB::memorypolicies::Single_Thread, BB::memorypolicies::No_BoundsCheck, BB::memorypolicies::No_MemoryTagging> unsafePoolAllocator_t;
}