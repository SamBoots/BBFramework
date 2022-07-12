#pragma once
#include <vector>
#include "Storage/Dynamic_Array.h"
#include "Storage/Pool.h"

namespace BB
{
	//--------------------------------------------------------
	// A slice is a non-owning reference to N contiguous elements in memory
	// Slice is a way to abstract sending dynamic_array's, vectors or stack arrays.
	//--------------------------------------------------------
	template<typename T>
	struct Slice
	{
		Slice(T* a_Ptr, size_t a_Count) : ptr(a_Ptr), count(a_Count) {};
		Slice(T* a_Begin, T* a_End) : ptr(a_Begin), count(a_End - a_Begin) {};
		Slice(std::vector<T>& a_Vector) : ptr(a_Vector.data()), count(a_Vector.size()) {};
		Slice(Dynamic_Array<T>& a_Array) : ptr(a_Array.data()), count(a_Array.size()) {};
		Slice(Pool<T>& a_Pool) : ptr(a_Pool.data()), count(a_Pool.size()) {};

		T& operator[](size_t a_Index)
		{
			BB_ASSERT(count > a_Index, "Slice error, trying to access memory");
			return ptr[a_Index];
		}

		Slice SubSlice(size_t a_Position, size_t a_Count)
		{
			BB_ASSERT(count > a_Position + a_Count - 1, "Subslice error, the subslice has unowned memory.");
			return Slice(ptr + a_Position, a_Count);
		}

		T* ptr;
		size_t count;
	};
}