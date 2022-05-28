#pragma once
#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)

#include "Hashmap.h"
#include "Utils/Math.h"

TEST(Hashmap_Datastructure, Hashmap_Insert)
{
	//Unaligned big struct with a union to test the value.
	struct size2593bytes { union { char data[2593]; size_t value; }; };

	//2 MB alloactor.
	const size_t allocatorSize = BB::gbSize * 2;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::HashMap<size2593bytes, const char*, BB::FreeListAllocator_t> t_Array(t_Allocator);
}