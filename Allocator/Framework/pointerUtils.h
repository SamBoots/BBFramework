#pragma once
#include <cstdint>

namespace BB
{
	namespace pointerutils
	{
		void* Add(void* a_Ptr, size_t a_Add)
		{
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a_Ptr) + a_Add);
		}
	}
}