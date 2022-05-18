#pragma once

#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)
#include "BB_AllocTypes.h"
#include "Utils/Math.h"

//Bytes samples with different sizes.
constexpr const size_t sample_32_bytes = 10000;
constexpr const size_t sample_256_bytes = 2000;
constexpr const size_t sample_2593_bytes = 1000;

//How many samples in total.
constexpr const size_t samples = sample_32_bytes + sample_256_bytes + sample_2593_bytes;

//Structs with different sizes, union to check for the values.
struct size32Bytes { union { char data[32]; size_t value; }; };
struct size256Bytes { union { char data[256]; size_t value; }; };
struct size2593bytes { union { char data[2593]; size_t value; }; };



#pragma region LINEAR_ALLOCATOR
TEST(MemoryAllocators, LINEAR_SINGLE_ALLOCATIONS)
{
	std::cout << "Linear allocator with 10000 32 byte samples, 2000 256 byte samples and 1000 2593 bytes samples." << "\n";

	constexpr const size_t allocatorSize = 
		sizeof(size32Bytes) * sample_32_bytes +
		sizeof(size256Bytes) * sample_256_bytes +
		sizeof(size2593bytes) * sample_2593_bytes;

	//Get some random values to test.
	size_t randomValues[samples]{};
	for (size_t i = 0; i < samples; i++)
	{
		randomValues[i] = static_cast<size_t>(Utils::RandomUInt());
	}

	BB::unsafeLinearAllocator_t t_LinearAllocator(allocatorSize);
	
	for (size_t i = 0; i < sample_32_bytes; i++)
	{
		size32Bytes* sample = BB::AllocNew<size32Bytes>(t_LinearAllocator);
		sample->value = randomValues[i];
	}
	for (size_t i = 0; i < sample_256_bytes; i++)
	{
		size256Bytes* sample = BB::AllocNew<size256Bytes>(t_LinearAllocator);
		sample->value = randomValues[sample_32_bytes + i];
	}
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		size2593bytes* sample = BB::AllocNew<size2593bytes>(t_LinearAllocator);
		sample->value = randomValues[sample_32_bytes + sample_256_bytes + i];
	}

	//Test all the values inside the allocations.
	void* t_AllocData = t_LinearAllocator.begin();
	for (size_t i = 0; i < sample_32_bytes; i++)
	{
		size32Bytes* data = reinterpret_cast<size32Bytes*>(t_AllocData);
		ASSERT_EQ(data->value, randomValues[i]) << "32 bytes, Value is different in the linear allocator.";
		t_AllocData = BB::pointerutils::Add(t_AllocData, sizeof(size32Bytes));
	}
	for (size_t i = sample_32_bytes; i < sample_32_bytes + sample_256_bytes; i++)
	{
		ASSERT_EQ(reinterpret_cast<size256Bytes*>(t_AllocData)->value, randomValues[i]) << "256 bytes, Value is different in the linear allocator.";
		t_AllocData = BB::pointerutils::Add(t_AllocData, sizeof(size256Bytes));
	}
	for (size_t i = sample_32_bytes + sample_256_bytes; i < sample_32_bytes + sample_256_bytes + sample_2593_bytes; i++)
	{
		ASSERT_EQ(reinterpret_cast<size2593bytes*>(t_AllocData)->value, randomValues[i]) << "2593 bytes, Value is different in the linear allocator.";
		t_AllocData = BB::pointerutils::Add(t_AllocData, sizeof(size2593bytes));
	}

	t_LinearAllocator.Clear();
}


TEST(MemoryAllocators, LINEAR_ARRAY_ALLOCATIONS)
{
	std::cout << "Linear allocator with 10000 32 byte samples, 2000 256 byte samples and 1000 2593 bytes samples." << "\n";

	constexpr const size_t allocatorSize =
		sizeof(size32Bytes) * sample_32_bytes +
		sizeof(size256Bytes) * sample_256_bytes +
		sizeof(size2593bytes) * sample_2593_bytes;

	//Get some random values to test.
	size_t randomValues[samples]{};
	for (size_t i = 0; i < samples; i++)
	{
		randomValues[i] = static_cast<size_t>(Utils::RandomUInt());
	}

	BB::unsafeLinearAllocator_t t_LinearAllocator(allocatorSize);

	size32Bytes* size32Array = BB::AllocNewArray<size32Bytes>(t_LinearAllocator, sample_32_bytes);
	size256Bytes* size256Array = BB::AllocNewArray<size256Bytes>(t_LinearAllocator, sample_256_bytes);
	size2593bytes* size2593Array = BB::AllocNewArray<size2593bytes>(t_LinearAllocator, sample_2593_bytes);

	//Checking the arrays
	for (size_t i = 0; i < sample_32_bytes; i++)
	{
		size32Array[i].value = randomValues[i];
	}
	for (size_t i = 0; i < sample_256_bytes; i++)
	{
		size256Array[i].value = randomValues[sample_32_bytes + i];
	}
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		size2593Array[i].value = randomValues[sample_32_bytes + sample_256_bytes + i];
	}

	//Checking the arrays
	for (size_t i = 0; i < sample_32_bytes; i++)
	{
		ASSERT_EQ(size32Array[i].value, randomValues[i]) << "32 bytes, Value is different in the linear allocator.";
	}
	for (size_t i = 0; i < sample_256_bytes; i++)
	{
		ASSERT_EQ(size256Array[i].value, randomValues[sample_32_bytes + i]) << "256 bytes, Value is different in the linear allocator.";
	}
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		ASSERT_EQ(size2593Array[i].value, randomValues[sample_32_bytes + sample_256_bytes + i]) << "2593 bytes, Value is different in the linear allocator.";
	}
}
#pragma endregion

#pragma region FREELIST_ALLOCATOR
TEST(MemoryAllocators, FREELIST_SINGLE_ALLOCATIONS)
{
	std::cout << "Freelist allocator with 10000 32 byte samples, 2000 256 byte samples and 1000 2593 bytes samples." << "\n";

	//Using union so that I can check if the data is correct.
	struct size32Bytes { union { char data[32]; uint64_t value; }; };
	struct size256Bytes { union { char data[256]; uint64_t value; }; };
	struct size2593bytes { union { char data[2593]; uint64_t value; }; };

	//allocator size is modified by the allocheader it needs.
	constexpr const size_t allocatorSize =
		(sizeof(size32Bytes) * sample_32_bytes +
		sizeof(size256Bytes) * sample_256_bytes +
		sizeof(size2593bytes) * sample_2593_bytes) * 
		sizeof(BB::allocators::FreelistAllocator::AllocHeader);

	//Get some random values to test.
	size_t randomValues[samples]{};
	for (size_t i = 0; i < samples; i++)
	{
		randomValues[i] = static_cast<size_t>(Utils::RandomUInt());
	}

	BB::unsafeFreeListAllocator_t t_FreelistAllocator(allocatorSize);

	{
		//This address should always be used since it's a free block.
		void* repeatAddress32 = BB::AllocNew<size32Bytes>(t_FreelistAllocator);
		BB::Free(t_FreelistAllocator, repeatAddress32);

		for (size_t i = 0; i < sample_32_bytes; i++)
		{
			size32Bytes* sample = BB::AllocNew<size32Bytes>(t_FreelistAllocator);
			sample->value = randomValues[i];
			//Compare the values.
			ASSERT_EQ(sample->value, randomValues[i]) << "32 bytes, Value is different in the freelist allocator.";
			//Compare the addresses.
			ASSERT_EQ(sample, repeatAddress32) << "32 bytes, address is different in the freelist allocator.";
			BB::Free(t_FreelistAllocator, sample);
		}
	}
	{
		//This address should always be used since it's a free block.
		void* repeatAddress256 = BB::AllocNew<size256Bytes>(t_FreelistAllocator);
		BB::Free(t_FreelistAllocator, repeatAddress256);

		for (size_t i = 0; i < sample_256_bytes; i++)
		{
			size256Bytes* sample = BB::AllocNew<size256Bytes>(t_FreelistAllocator);
			sample->value = randomValues[sample_32_bytes + i];
			//Compare the values.
			ASSERT_EQ(sample->value, randomValues[sample_32_bytes + i]) << "256 bytes, Value is different in the freelist allocator.";
			//Compare the addresses.
			ASSERT_EQ(sample, repeatAddress256) << "256 bytes, address is different in the freelist allocator.";
			BB::Free(t_FreelistAllocator, sample);
		}
	}
	{
		//This address should always be used since it's a free block.
		void* repeatAddress2593 = BB::AllocNew<size2593bytes>(t_FreelistAllocator);
		BB::Free(t_FreelistAllocator, repeatAddress2593);

		for (size_t i = 0; i < sample_2593_bytes; i++)
		{
			size2593bytes* sample = BB::AllocNew<size2593bytes>(t_FreelistAllocator);
			sample->value = randomValues[sample_32_bytes + sample_256_bytes + i];
			//Compare the values.
			ASSERT_EQ(sample->value, randomValues[sample_32_bytes + sample_256_bytes + i]) << "2593 bytes, Value is different in the freelist allocator.";
			//Compare the addresses.
			ASSERT_EQ(sample, repeatAddress2593) << "2593 bytes, address is different in the freelist allocator.";
			BB::Free(t_FreelistAllocator, sample);
		}
	}
	//Clear is not suppoted by freelist, commented just to show this is not a mistake.
	//t_FreelistAllocator.Clear();
}


TEST(MemoryAllocators, FREELIST_ARRAY_ALLOCATIONS)
{
	std::cout << "Freelist allocator with 10000 32 byte samples, 2000 256 byte samples and 1000 2593 bytes samples." << "\n";

	constexpr const size_t allocatorSize =
		(sizeof(size32Bytes) * sample_32_bytes +
			sizeof(size256Bytes) * sample_256_bytes +
			sizeof(size2593bytes) * sample_2593_bytes) * 2;

	//Get some random values to test.
	size_t randomValues[samples]{};
	for (size_t i = 0; i < samples; i++)
	{
		randomValues[i] = static_cast<size_t>(Utils::RandomUInt());
	}

	BB::unsafeFreeListAllocator_t t_FreeList(allocatorSize);

	size32Bytes* size32Array = BB::AllocNewArray<size32Bytes>(t_FreeList, sample_32_bytes);
	size256Bytes* size256Array = BB::AllocNewArray<size256Bytes>(t_FreeList, sample_256_bytes);
	size2593bytes* size2593Array = BB::AllocNewArray<size2593bytes>(t_FreeList, sample_2593_bytes);

	//Checking the arrays
	for (size_t i = 0; i < sample_32_bytes; i++)
	{
		size32Array[i].value = randomValues[i];
	}
	for (size_t i = 0; i < sample_256_bytes; i++)
	{
		size256Array[i].value = randomValues[sample_32_bytes + i];
	}
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		size2593Array[i].value = randomValues[sample_32_bytes + sample_256_bytes + i];
	}

	//Checking the arrays
	for (size_t i = 0; i < sample_32_bytes; i++)
	{
		ASSERT_EQ(size32Array[i].value, randomValues[i]) << "32 bytes, Value is different in the freelist allocator.";
	}
	for (size_t i = 0; i < sample_256_bytes; i++)
	{
		ASSERT_EQ(size256Array[i].value, randomValues[sample_32_bytes + i]) << "256 bytes, Value is different in the freelist allocator.";
	}
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		ASSERT_EQ(size2593Array[i].value, randomValues[sample_32_bytes + sample_256_bytes + i]) << "2593 bytes, Value is different in the freelist allocator.";
	}

	//Clear is not suppoted by freelist, commented just to show this is not a mistake.
	//t_FreelistAllocator.Clear();
}


TEST(MemoryAllocators, FREELIST_RESIZE_MEMSET)
{
#ifdef _X64
	//1 GB allocation
	constexpr const size_t allocatorSize = 1073741824;
#endif
#ifdef _X86
	//64 MB allocation for X86 due to the virtual address limit and a 4 bytes unsigned int limit.
	constexpr const size_t allocatorSize = 16777216;
#endif

	struct AlignmentCheckStruct { char data[allocatorSize]; };

	BB::allocators::FreelistAllocator t_FreeList(allocatorSize + sizeof(BB::allocators::FreelistAllocator::AllocHeader));

	//Alloc the entire thing, memset to check if the allocation throws errors.
	void* t_Data = t_FreeList.Alloc(allocatorSize, __alignof(AlignmentCheckStruct));
	memset(t_Data, 256, allocatorSize);

	ASSERT_EQ(t_FreeList.m_FreeBlocks, nullptr) << "Allocator resized while it shouldn't.";

	t_Data = t_FreeList.Alloc(allocatorSize, __alignof(AlignmentCheckStruct));
	memset(t_Data, 256, allocatorSize);

	ASSERT_EQ(t_FreeList.m_FreeBlocks, nullptr) << "Allocator resized, but there is a freeblock while there shouldn't be one available. Since the allocator should be totally full.";


	//Clear is not suppoted by freelist, commented just to show this is not a mistake.
	//t_FreelistAllocator.Clear();
}

#pragma endregion

#pragma region POOL_ALLOCATOR
TEST(MemoryAllocators, POOL_SINGLE_ALLOCATIONS)
{
	std::cout << "Pool allocator with 1000 2593 bytes samples." << "\n";

	//Get some random values to test
	size_t randomValues[sample_2593_bytes]{};
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		randomValues[i] = static_cast<size_t>(Utils::RandomUInt());
	}

	BB::unsafePoolAllocator_t t_PoolAllocator(sizeof(size2593bytes), sample_2593_bytes, __alignof(size2593bytes));

	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		size2593bytes* sample = BB::AllocNew<size2593bytes>(t_PoolAllocator);
		sample->value = randomValues[i];
	}

	//Test all the values inside the allocations.
	size2593bytes* t_AllocData = reinterpret_cast<size2593bytes*>(t_PoolAllocator.begin());
	for (size_t i = 0; i < sample_2593_bytes; i++)
	{
		ASSERT_EQ(t_AllocData[i].value, randomValues[i]) << "2593 bytes, Value is different in the Pool allocator.";

	}

	t_PoolAllocator.Clear();
}

TEST(MemoryAllocators, POOL_SINGLE_ALLOCATIONS_RESIZE)
{
	//std::cout << "Pool allocator with 1000 2593 bytes samples, but an allocator size for only 100 elements to test resizing." << "\n";

	////Get some random values to test
	//size_t randomValues[sample_2593_bytes]{};
	//for (size_t i = 0; i < sample_2593_bytes; i++)
	//{
	//	randomValues[i] = static_cast<size_t>(Utils::RandomUInt());
	//}

	//BB::unsafePoolAllocator_t t_PoolAllocator(sizeof(size2593bytes), sample_2593_bytes / 10, __alignof(size2593bytes));

	//for (size_t i = 0; i < sample_2593_bytes; i++)
	//{
	//	size2593bytes* sample = BB::AllocNew<size2593bytes>(t_PoolAllocator);
	//	sample->value = randomValues[i];
	//}

	////Test all the values inside the allocations.
	//size2593bytes* t_AllocData = reinterpret_cast<size2593bytes*>(t_PoolAllocator.begin());
	//for (size_t i = 0; i < sample_2593_bytes; i++)
	//{
	//	ASSERT_EQ(t_AllocData[i].value, randomValues[i]) << "2593 bytes, Value is different in the Pool allocator.";

	//}

	//t_PoolAllocator.Clear();
}

#pragma endregion