#pragma once
#include <cstdint>

namespace BB
{
	namespace pointerutils
	{
		static void* Add(void* a_Ptr, size_t a_Add)
		{
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a_Ptr) + a_Add);
		}

		static size_t alignForwardAdjustment(const void* address, size_t alignment)
		{
			size_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(alignment - 1));

			if (adjustment == alignment) return 0;

			//already aligned 
			return adjustment;
		}

		static size_t alignForwardAdjustmentHeader(const void* address, size_t alignment, size_t headerSize)
		{
			size_t adjustment = alignForwardAdjustment(address, alignment);
			size_t neededSpace = headerSize;

			if (adjustment < neededSpace)
			{
				neededSpace -= adjustment;

				//Increase adjustment to fit header 
				adjustment += alignment * (neededSpace / alignment);

				if (neededSpace % alignment > 0) adjustment += alignment;
			}

			return adjustment;
		}
	}
}