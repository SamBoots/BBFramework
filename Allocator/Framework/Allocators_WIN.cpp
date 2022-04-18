#include "pch.h"
#include "Allocators.h"
#include <stdlib.h> 

using namespace BB::allocators;


static uint8_t* alignForward(void* address, uint8_t alignment)
{
	return (uint8_t*)((reinterpret_cast<uintptr_t>(address) + static_cast<uintptr_t>(alignment - 1)) & static_cast<uintptr_t>(~(alignment - 1)));
}

static uint8_t alignForwardAdjustment(const void* address, uint8_t alignment)
{
	uint8_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(alignment - 1));

	if (adjustment == alignment) return 0;

	//already aligned 
	return adjustment;
}

static uint8_t alignForwardAdjustmentWithHeader(const void* address, uint8_t alignment, uint8_t headerSize)
{
	uint8_t adjustment = alignForwardAdjustment(address, alignment);
	uint8_t neededSpace = headerSize;

	if (adjustment < neededSpace)
	{
		neededSpace -= adjustment;

		//Increase adjustment to fit header 
		adjustment += alignment * (neededSpace / alignment);

		if (neededSpace % alignment > 0) adjustment += alignment;
	}

	return adjustment;
}

LinearAllocator::LinearAllocator(const size_t a_Size) : m_Size(a_Size)
{
	ASSERT(a_Size != 0, "linear alloc size is 0!");
	m_Start = reinterpret_cast<uint8_t*>(malloc(m_Size));
	m_Buffer = m_Start;
}

LinearAllocator::~LinearAllocator()
{
	free(m_Start);
}

void* LinearAllocator::Alloc(size_t a_Size, uint8_t a_Alignment)
{
	uint8_t t_Adjustment = alignForwardAdjustment(m_Buffer, a_Alignment);

	uintptr_t t_Address = reinterpret_cast<uintptr_t>(m_Buffer + t_Adjustment);
	m_Buffer = reinterpret_cast<uint8_t*>(t_Address + a_Size);

	return reinterpret_cast<void*>(t_Address);
}

void LinearAllocator::Clear()
{
	m_Buffer = m_Start;
}
