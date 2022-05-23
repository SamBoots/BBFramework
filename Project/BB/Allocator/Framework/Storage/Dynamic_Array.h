#pragma once
#include "pch.h"
#include "Framework/Allocators/BB_AllocTypes.h"

#include "Utils/Math.h"

namespace BB
{
	namespace Dynamic_Array_Specs
	{
		constexpr const size_t overAllocateMultiplier = 16;
		constexpr const size_t multipleValue = 8;
	};

	template<typename T, typename Allocator>
	struct Dynamic_Array
	{
		Dynamic_Array(Allocator& a_Allocator);
		Dynamic_Array(Allocator& a_Allocator, size_t a_Size);
		~Dynamic_Array();

		T& operator[](const size_t a_Index) const;

		void push_back(T& a_Element);
		void push_back(const T* a_Elements, size_t a_Count);
		void reserve(size_t a_Size);

		void pop();
		void empty();

		const size_t size() const { return m_Size; };
		const size_t capacity() const { return m_Capacity; }
		const void* data() const { return m_Allocator.begin(); };

	private:

		void grow(size_t a_MinCapacity = 0);
		//This function also changes the m_Capacity value.
		void reallocate(size_t a_NewCapacity);

		Allocator& m_Allocator;

		T* m_Arr;
		size_t m_Size = 0;
		size_t m_Capacity;
	};

	template<typename T, typename Allocator>
	inline BB::Dynamic_Array<T, Allocator>::Dynamic_Array(Allocator& a_Allocator)
		: m_Allocator(a_Allocator)
	{
		m_Capacity = Dynamic_Array_Specs::multipleValue;

		m_Arr = BBallocArray<T>(m_Allocator, m_Capacity);
	}

	template<typename T, typename Allocator>
	inline Dynamic_Array<T, Allocator>::Dynamic_Array(Allocator& a_Allocator, size_t a_Size)
		: m_Allocator(a_Allocator)
	{
		BB_EXCEPTION(a_Size != 0, "Dynamic_array size is specified to be 0, use constructor without size!");
		m_Capacity = Math::RoundUp(a_Size, Dynamic_Array_Specs::multipleValue);

		m_Arr = BBallocArray<T>(m_Allocator, m_Capacity);
	}

	template<typename T, typename Allocator>
	inline Dynamic_Array<T, Allocator>::~Dynamic_Array()
	{
		BBFreeArray<T>(m_Allocator, m_Arr);
	}

	template<typename T, typename Allocator>
	inline T& Dynamic_Array<T, Allocator>::operator[](const size_t a_Index) const
	{
		BB_EXCEPTION(a_Index <= m_Size, "Dynamic_Array, trying to get an element using the [] operator but that element is not there.");
		return m_Arr[a_Index];
	}

	template<typename T, typename Allocator>
	inline void Dynamic_Array<T, Allocator>::push_back(T& a_Element)
	{
		if (m_Size >= m_Capacity)
			grow();

		m_Arr[m_Size] = a_Element;
		m_Size++;
	}

	template<typename T, typename Allocator>
	inline void Dynamic_Array<T, Allocator>::push_back(const T* a_Elements, size_t a_Count)
	{
		if (m_Size + a_Count >= m_Capacity)
			grow();

		memcpy(m_Arr + m_Size, a_Elements, sizeof(T) * a_Count);
		m_Size += a_Count;
	}

	template<typename T, typename Allocator>
	inline void Dynamic_Array<T, Allocator>::reserve(size_t a_Size)
	{
		if (a_Size > m_Capacity)
		{
			size_t t_ModifiedCapacity = Math::RoundUp(a_Size, Dynamic_Array_Specs::multipleValue);

			reallocate(t_ModifiedCapacity);
			return;
		}
	}

	template<typename T, typename Allocator>
	inline void BB::Dynamic_Array<T, Allocator>::pop()
	{
		m_Size--;
	}

	template<typename T, typename Allocator>
	inline void BB::Dynamic_Array<T, Allocator>::empty()
	{
		m_Size = 0;
	}

	template<typename T, typename Allocator>
	inline void Dynamic_Array<T, Allocator>::grow(size_t a_MinCapacity)
	{
		size_t t_ModifiedCapacity = m_Capacity * 2;

		if (a_MinCapacity > t_ModifiedCapacity)
			t_ModifiedCapacity = Math::RoundUp(t_ModifiedCapacity, Dynamic_Array_Specs::multipleValue);

		reallocate(t_ModifiedCapacity);
	}

	template<typename T, typename Allocator>
	inline void Dynamic_Array<T, Allocator>::reallocate(size_t a_NewCapacity)
	{
		T* t_NewArr = BBallocArray<T>(m_Allocator, a_NewCapacity);
		memcpy(t_NewArr, m_Arr, sizeof(T) * m_Size);

		BBFreeArray(m_Allocator, m_Arr);

		m_Arr = t_NewArr;

		m_Capacity = a_NewCapacity;
	}
}