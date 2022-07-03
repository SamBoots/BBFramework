#pragma once
#include "pch.h"
#include "Allocators/AllocTypes.h"

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
		struct Iterator
		{
			//Iterator idea from:
			//https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp

			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;

			Iterator(pointer a_Ptr) : m_Ptr(a_Ptr) {}

			reference operator*() const { return *m_Ptr; }
			pointer operator->() { return m_Ptr; }

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

			friend bool operator== (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_ptr == a_Rhs.m_ptr; };
			friend bool operator!= (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_ptr != a_Rhs.m_ptr; };

		private:
			pointer m_Ptr;
		};

		Dynamic_Array(Allocator& a_Allocator);
		Dynamic_Array(Allocator& a_Allocator, size_t a_Size);
		~Dynamic_Array();

		T& operator[](const size_t a_Index) const;

		void push_back(T& a_Element);
		void push_back(const T* a_Elements, size_t a_Count);
		void reserve(size_t a_Size);

		void insert(size_t a_Position, const T& a_Element);
		void insert(size_t a_Position, const T* a_Elements, size_t a_Count);

		void pop();
		void empty();

		const size_t size() const { return m_Size; };
		const size_t capacity() const { return m_Capacity; }
		const void* data() const { return m_Allocator.begin(); };

		Iterator begin() { return Iterator(m_Arr); }
		Iterator end() { return Iterator(m_Arr(m_Size + 1)); } //Get an out of bounds Iterator.
			 
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
		if (m_Size + a_Count > m_Capacity)
			grow(a_Count);

		memcpy(&m_Arr[m_Size], a_Elements, sizeof(T) * a_Count);
		m_Size += a_Count;
	}

	template<typename T, typename Allocator>
	inline void BB::Dynamic_Array<T, Allocator>::insert(size_t a_Position, const T& a_Element)
	{
		BB_ASSERT(m_Size >= a_Position, "trying to insert in a position that is bigger then the current Dynamic_Array size!");
		if (m_Size >= m_Capacity)
			grow();

		//Move all elements after a_Position 1 to the front.
		memcpy(&m_Arr[a_Position + 1], &m_Arr[a_Position], sizeof(T) * (m_Size - a_Position));

		m_Arr[a_Position] = a_Element;
		m_Size++;
	}

	template<typename T, typename Allocator>
	inline void BB::Dynamic_Array<T, Allocator>::insert(size_t a_Position, const T* a_Elements, size_t a_Count)
	{
		BB_ASSERT(m_Size >= a_Position, "trying to insert in a position that is bigger then the current Dynamic_Array size! Resize the array before ");
		if (m_Size + a_Count > m_Capacity)
			grow(a_Count);
		
		//Move all elements after a_Position 1 to the front.
		memcpy(&m_Arr[a_Position + a_Count], &m_Arr[a_Position], sizeof(T) * (m_Size - a_Position));

		//Set all the elements.
		memcpy(&m_Arr[a_Position], a_Elements, sizeof(T) * a_Count);
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
			t_ModifiedCapacity = Math::RoundUp(a_MinCapacity, Dynamic_Array_Specs::multipleValue);

		reallocate(t_ModifiedCapacity);
	}

	template<typename T, typename Allocator>
	inline void Dynamic_Array<T, Allocator>::reallocate(size_t a_NewCapacity)
	{
		T* t_NewArr = BBallocArray<T>(m_Allocator, a_NewCapacity);
		memcpy(t_NewArr, m_Arr, sizeof(T) * m_Capacity);

		BBFreeArray(m_Allocator, m_Arr);

		m_Arr = t_NewArr;

		m_Capacity = a_NewCapacity;
	}
}