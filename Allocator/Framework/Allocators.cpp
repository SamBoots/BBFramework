#include "pch.h"
#include "Allocators.h"
#include "pointerUtils.h"

#include "BackingAllocator.h"

using namespace BB::allocators;

LinearAllocator::LinearAllocator(const size_t a_Size)
{
	BB_ASSERT(a_Size != 0, "linear allocator is created with a size of 0!");
	m_Start = reinterpret_cast<uint8_t*>(virtualAllocBackingAllocator.Alloc(a_Size));
	m_Buffer = m_Start;
}

LinearAllocator::~LinearAllocator()
{
	//free(m_Start);
}

void* LinearAllocator::Alloc(size_t a_Size, size_t a_Alignment)
{
	size_t t_Adjustment = pointerutils::alignForwardAdjustment(m_Buffer, a_Alignment);

	uintptr_t t_Address = reinterpret_cast<uintptr_t>(m_Buffer + t_Adjustment);
	m_Buffer = reinterpret_cast<uint8_t*>(t_Address + a_Size);

	return reinterpret_cast<void*>(t_Address);
}

void LinearAllocator::Free(void*)
{
	BB_EXCEPTION(false, "Tried to free a linear allocator, which is not possible!");
}

void LinearAllocator::Clear()
{
	m_Buffer = m_Start;
}


FreelistAllocator::FreelistAllocator(const size_t a_Size)
{
	BB_ASSERT(a_Size != 0, "Freelist allocator is created with a size of 0!");
	BB_WARNING(a_Size > 10240, "Freelist allocator is smaller then 10 kb, might be too small.");
	m_Start = reinterpret_cast<uint8_t*>(virtualAllocBackingAllocator.Alloc(a_Size));
	m_FreeBlocks = reinterpret_cast<FreeBlock*>(m_Start);
	m_FreeBlocks->size = a_Size;
	m_FreeBlocks->next = nullptr;
}

FreelistAllocator::~FreelistAllocator()
{
	//free(m_Start);
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

		//BB_STATIC_ASSERT(sizeof(AllocHeader) >= sizeof(FreeBlock), "sizeof(AllocationHeader) < sizeof(FreeBlock)");

		{
			FreeBlock* t_NextBlock = reinterpret_cast<FreeBlock*>(pointerutils::Add(t_FreeBlock, t_TotalSize));

			t_NextBlock->size = t_FreeBlock->size - t_TotalSize;
			t_NextBlock->next = t_FreeBlock->next;

			if (t_PreviousFreeBlock != nullptr)
				t_PreviousFreeBlock->next = t_NextBlock;
			else
				m_FreeBlocks = t_NextBlock;
		}

		uintptr_t t_Address = reinterpret_cast<uintptr_t>(t_FreeBlock + t_Adjustment);
		AllocHeader* t_Header = reinterpret_cast<AllocHeader*>(t_Address - sizeof(AllocHeader));
		t_Header->size = t_TotalSize;
		t_Header->adjustment = t_Adjustment;

		return reinterpret_cast<void*>(t_Address);
	}

	BB_ASSERT(false, "Freelist allocator out of memory!");
	return nullptr;
}

void FreelistAllocator::Free(void* a_Ptr)
{

}
