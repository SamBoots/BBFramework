#pragma once
#include <cstdint>

namespace BB
{
	namespace pointerutils
	{
		/// <summary>
		/// Move the given pointer by a given size.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to shift </param>
		/// <param name="a_Add:"> The amount of bytes you want move the pointer forward. </param>
		/// <returns>The shifted pointer. </returns>
		inline static void* Add(void* a_Ptr, size_t a_Add)
		{
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a_Ptr) + a_Add);
		}

		/// <summary>
		/// Move the given pointer by a given size.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to shift </param>
		/// <param name="a_Subtract:"> The amount of bytes you want move the pointer backwards. </param>
		/// <returns>The shifted pointer. </returns>
		inline static void* Subtract(void* a_Ptr, size_t a_Subtract)
		{
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a_Ptr) - a_Subtract);
		}

		/// <summary>
		/// Align a given pointer forward.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to align </param>
		/// <param name="a_Alignment:"> The alignment of the data. </param>
		/// <returns>The given address but aligned forward. </returns>
		inline static size_t alignForwardAdjustment(const void* a_Ptr, size_t a_Alignment)
		{
			size_t adjustment = a_Alignment - (reinterpret_cast<uintptr_t>(a_Ptr) & static_cast<uintptr_t>(a_Alignment - 1));

			if (adjustment == a_Alignment) return 0;

			//already aligned 
			return adjustment;
		}

		/// <summary>
		/// Align a given pointer forward.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to align </param>
		/// <param name="a_Alignment:"> The alignment of the data. </param>
		/// <param name="a_HeaderSize:"> The size in bytes of the Header you want to align forward's too </param>
		/// <returns>The given address but aligned forward with the allocation header's size in mind. </returns>
		inline static size_t alignForwardAdjustmentHeader(const void* a_Ptr, size_t a_Alignment, size_t a_HeaderSize)
		{
			size_t adjustment = alignForwardAdjustment(a_Ptr, a_Alignment);
			size_t neededSpace = a_HeaderSize;

			if (adjustment < neededSpace)
			{
				neededSpace -= adjustment;

				//Increase adjustment to fit header 
				adjustment += a_Alignment * (neededSpace / a_Alignment);

				if (neededSpace % a_Alignment > 0) adjustment += a_Alignment;
			}

			return adjustment;
		}
	}
}