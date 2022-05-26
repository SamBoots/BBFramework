#include "pch.h"
#include "Allocators.h"
#include "Utils/pointerUtils.h"

#include "BackingAllocator.h"

using namespace BB::allocators;

LinearAllocator::LinearAllocator(const size_t a_Size)
{
	BB_ASSERT(a_Size != 0, "linear allocator is created with a size of 0!");
	m_Start = mallocVirtual(nullptr, a_Size);
	m_Buffer = m_Start;
	m_End = reinterpret_cast<uintptr_t>(m_Start) + a_Size;
}

LinearAllocator::~LinearAllocator()
{
	freeVirtual(reinterpret_cast<void*>(m_Start));
}

void* LinearAllocator::Alloc(size_t a_Size, size_t a_Alignment)
{
	size_t t_Adjustment = pointerutils::alignForwardAdjustment(m_Buffer, a_Alignment);

	uintptr_t t_Address = reinterpret_cast<uintptr_t>(pointerutils::Add(m_Buffer, t_Adjustment));
	m_Buffer = reinterpret_cast<void*>(t_Address + a_Size);

	if (t_Address > m_End)
	{
		size_t t_Increase = m_End - reinterpret_cast<uintptr_t>(m_Start);
		mallocVirtual(m_Start, t_Increase);
		m_End += t_Increase;
	}
		

	return reinterpret_cast<void*>(t_Address);
}

void LinearAllocator::Free(void*)
{
	BB_ASSERT(false, "Tried to free a piece of memory in a linear allocator, which is not possible!");
}

void LinearAllocator::Clear()
{
	m_Buffer = m_Start;
}


FreelistAllocator::FreelistAllocator(const size_t a_Size)
{
	BB_ASSERT(a_Size != 0, "Freelist allocator is created with a size of 0!");
	BB_WARNING(a_Size > 10240, "Freelist allocator is smaller then 10 kb, you generally want a bigger freelist.");
	m_TotalAllocSize = a_Size;
	m_Start = reinterpret_cast<uint8_t*>(mallocVirtual(nullptr, m_TotalAllocSize));
	m_FreeBlocks = reinterpret_cast<FreeBlock*>(m_Start);
	m_FreeBlocks->size = m_TotalAllocSize;
	m_FreeBlocks->next = nullptr;
}

FreelistAllocator::~FreelistAllocator()
{
	freeVirtual(m_Start);
}

void* FreelistAllocator::Alloc(size_t a_Size, size_t a_Alignment)
{
	FreeBlock* t_PreviousFreeBlock = nullptr;
	FreeBlock* t_FreeBlock = m_FreeBlocks;

	while (t_FreeBlock != nullptr)
	{
		size_t t_Adjustment = pointerutils::alignForwardAdjustmentHeader(t_FreeBlock, a_Alignment, sizeof(AllocHeader));
		size_t t_TotalSize = a_Size + t_Adjustment;

		if (t_FreeBlock->size < t_TotalSize)
		{
			t_PreviousFreeBlock = t_FreeBlock;
			t_FreeBlock = t_FreeBlock->next;
			continue;
		}

		if (t_FreeBlock->size - t_TotalSize <= sizeof(AllocHeader))
		{
			t_TotalSize = t_FreeBlock->size;

			if (t_PreviousFreeBlock != nullptr)
				t_PreviousFreeBlock->next = t_FreeBlock->next;
			else
				m_FreeBlocks = t_FreeBlock->next;
		}
		else
		{
			FreeBlock* t_NextBlock = reinterpret_cast<FreeBlock*>(pointerutils::Add(t_FreeBlock, t_TotalSize));

			t_NextBlock->size = t_FreeBlock->size - t_TotalSize;
			t_NextBlock->next = t_FreeBlock->next;

			if (t_PreviousFreeBlock != nullptr)
				t_PreviousFreeBlock->next = t_NextBlock;
			else
				m_FreeBlocks = t_NextBlock;
		}

		uintptr_t t_Address = reinterpret_cast<uintptr_t>(t_FreeBlock) + t_Adjustment;
		AllocHeader* t_Header = reinterpret_cast<AllocHeader*>(t_Address - sizeof(AllocHeader));
		t_Header->size = t_TotalSize;
		t_Header->adjustment = t_Adjustment;

		return reinterpret_cast<void*>(t_Address);
	}
	BB_WARNING(false, "Increasing the size of a freelist allocator, risk of fragmented memory.");
	//Double the size of the freelist.
	FreeBlock* t_NewAllocBlock = reinterpret_cast<FreeBlock*>(mallocVirtual(m_Start, m_TotalAllocSize));
	t_NewAllocBlock->size = m_TotalAllocSize;
	t_NewAllocBlock->next = m_FreeBlocks;

	//Update the new total alloc size.
	m_TotalAllocSize += m_TotalAllocSize;

	//Set the new block as the main block.
	m_FreeBlocks = t_NewAllocBlock;

	return this->Alloc(a_Size, a_Alignment);
}

void FreelistAllocator::Free(void* a_Ptr)
{
	BB_ASSERT(a_Ptr != nullptr, "Nullptr send to FreelistAllocator::Free!.");
	AllocHeader* t_Header = reinterpret_cast<AllocHeader*>(pointerutils::Subtract(a_Ptr, sizeof(AllocHeader)));
	size_t t_BlockSize = t_Header->size;
	uintptr_t t_BlockStart = reinterpret_cast<uintptr_t>(a_Ptr) - t_Header->adjustment;
	uintptr_t t_BlockEnd = t_BlockStart + t_BlockSize;

	FreeBlock* t_PreviousBlock = nullptr;
	FreeBlock* t_FreeBlock = m_FreeBlocks;

	while (t_FreeBlock != nullptr)
	{
		BB_ASSERT(t_FreeBlock != t_FreeBlock->next, "Next points to it's self.");
		uintptr_t t_FreeBlockPos = reinterpret_cast<uintptr_t>(t_FreeBlock);
		if (t_FreeBlockPos >= t_BlockEnd) break;
		t_PreviousBlock = t_FreeBlock;
		t_FreeBlock = t_FreeBlock->next;
	}

	if (t_PreviousBlock == nullptr)
	{
		t_PreviousBlock = reinterpret_cast<FreeBlock*>(t_BlockStart);
		t_PreviousBlock->size = t_Header->size;
		t_PreviousBlock->next = m_FreeBlocks;
		m_FreeBlocks = t_PreviousBlock;
	}
	else if (reinterpret_cast<uintptr_t>(t_PreviousBlock) + t_PreviousBlock->size == t_BlockStart)
	{
		t_PreviousBlock->size += t_BlockSize;
	}
	else
	{
		FreeBlock* t_Temp = reinterpret_cast<FreeBlock*>(t_BlockStart);
		t_Temp->size = t_BlockSize;
		t_Temp->next = t_PreviousBlock->next;
		t_PreviousBlock->next = t_Temp;
		t_PreviousBlock = t_Temp;
	}

	if (t_FreeBlock != nullptr && reinterpret_cast<uintptr_t>(t_FreeBlock) == t_BlockEnd)
	{
		t_PreviousBlock->size += t_FreeBlock->size;
		t_PreviousBlock->next = t_FreeBlock->next;
	}
}

void BB::allocators::FreelistAllocator::Clear() const
{
	BB_ASSERT(false, "Freelist allocator is not meant to be cleared yet.");
}


//BB::allocators::PoolAllocator::PoolAllocator(const size_t a_ObjectSize, const size_t a_ObjectCount, const size_t a_Alignment)
//{
//	BB_ASSERT(a_ObjectSize != 0, "Pool allocator is created with an object size of 0!");
//	BB_ASSERT(a_ObjectCount != 0, "Pool allocator is created with an object count of 0!");
//	//BB_WARNING(a_ObjectSize * a_ObjectCount > 10240, "Pool allocator is smaller then 10 kb, might be too small.");
//
//	size_t t_PoolAllocSize = a_ObjectSize * a_ObjectCount;
//	m_ObjectCount = a_ObjectCount;
//	m_Start = reinterpret_cast<void**>(mallocVirtual(m_Start, t_PoolAllocSize));
//	m_Alignment = pointerutils::alignForwardAdjustment(m_Start, a_Alignment);
//	m_Start = reinterpret_cast<void**>(pointerutils::Add(m_Start, m_Alignment));
//	m_Pool = m_Start;
//
//	void** t_Pool = m_Pool;
//
//	for (size_t i = 0; i < m_ObjectCount - 1; i++)
//	{
//		*t_Pool = pointerutils::Add(t_Pool, a_ObjectSize);
//		t_Pool = reinterpret_cast<void**>(*t_Pool);
//	}
//	*t_Pool = nullptr;
//}
//
//BB::allocators::PoolAllocator::~PoolAllocator()
//{
//	freeVirtual(m_Start);
//}
//
//void* BB::allocators::PoolAllocator::Alloc(size_t a_Size, size_t)
//{
//	void* t_Item = m_Pool;
//
//	//Increase the Pool allocator by double
//	if (t_Item == nullptr)
//	{
//		size_t t_Increase = m_ObjectCount;
//		size_t t_ByteIncrease = m_ObjectCount * a_Size;
//		mallocVirtual(m_Start, t_ByteIncrease);
//		void** t_Pool = reinterpret_cast<void**>(pointerutils::Add(m_Start, m_ObjectCount * a_Size + m_Alignment));
//		m_Pool = t_Pool;
//		m_ObjectCount += m_ObjectCount;
//		
//		for (size_t i = 0; i < t_Increase - 1; i++)
//		{
//			*t_Pool = pointerutils::Add(t_Pool, a_Size);
//			t_Pool = reinterpret_cast<void**>(*t_Pool);
//		}
//
//		t_Pool = nullptr;
//		return Alloc(a_Size, 0);
//	}
//	m_Pool = reinterpret_cast<void**>(*m_Pool);
//	return t_Item;
//}
//
//void BB::allocators::PoolAllocator::Free(void* a_Ptr)
//{
//	(*reinterpret_cast<void**>(a_Ptr)) = m_Pool;
//	m_Pool = reinterpret_cast<void**>(a_Ptr);
//}
//
//void BB::allocators::PoolAllocator::Clear()
//{
//	m_Pool = reinterpret_cast<void**>(m_Start);
//}
