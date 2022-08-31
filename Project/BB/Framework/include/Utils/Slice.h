#pragma once
#include <vector>
#include "Storage/Array.h"
#include "Storage/Pool.h"

namespace BB
{
	//--------------------------------------------------------
	// A slice is a non-owning reference to N contiguous elements in memory
	// Slice is a way to abstract sending dynamic_array's, vectors or stack arrays.
	//--------------------------------------------------------
	template<typename T>
	class Slice
	{
	public:
		struct Iterator
		{
			Iterator(T* a_Ptr) : m_Ptr(a_Ptr) {}

			T& operator*() const { return *m_Ptr; }
			T* operator->() { return m_Ptr; }

			Iterator& operator++()
			{
				m_Ptr++;
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator t_Tmp = *this;
				++(*this);
				return t_Tmp;
			}

			friend bool operator== (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr == a_Rhs.m_Ptr; };
			friend bool operator!= (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr != a_Rhs.m_Ptr; };

			friend bool operator< (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr < a_Rhs.m_Ptr; };
			friend bool operator> (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr > a_Rhs.m_Ptr; };
			friend bool operator<= (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr <= a_Rhs.m_Ptr; };
			friend bool operator>= (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr >= a_Rhs.m_Ptr; };

		private:
			T* m_Ptr;
		};

		Slice(T* a_Ptr, size_t a_Count) : m_Ptr(a_Ptr), m_Count(a_Count) {};
		Slice(T* a_Begin, T* a_End) : m_Ptr(a_Begin), m_Count(a_End - a_Begin) {};
		Slice(std::vector<T>& a_Vector) : m_Ptr(a_Vector.data()), m_Count(a_Vector.size()) {};
		Slice(Array<T>& a_Array) : m_Ptr(a_Array.data()), m_Count(a_Array.size()) {};
		Slice(Pool<T>& a_Pool) : m_Ptr(a_Pool.data()), m_Count(a_Pool.size()) {};

		T& operator[](size_t a_Index)
		{
			BB_ASSERT(m_Count > a_Index, "Slice error, trying to access memory");
			return m_Ptr[a_Index];
		}

		Slice SubSlice(size_t a_Position, size_t a_Count)
		{
			BB_ASSERT(m_Count > a_Position + a_Count - 1, "Subslice error, the subslice has unowned memory.");
			return Slice(m_Ptr + a_Position, a_Count);
		}

		Iterator begin() { return Iterator(m_Ptr); }
		Iterator end() { return Iterator(&m_Ptr[m_Count]); }

		T* data() { return m_Ptr; };
		size_t size() { return m_Count; }

	private:
		T* m_Ptr;
		size_t m_Count;
	};
}