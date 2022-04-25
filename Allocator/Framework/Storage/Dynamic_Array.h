#pragma once
#include "../BB_AllocTypes.h"

namespace BB
{
	template<typename T>
	struct Dynamic_Array
	{
		Dynamic_Array();
		~Dynamic_Array();

		T& operator[](const size_t a_Index) const;

		void push_back(T& a_Element);
		void resize(size_t a_Capacity);


		const size_t size() const { return m_Size; };
		const void* data() const { return m_Allocator.begin(); };

	private:

		size_t m_Size = 0;
		size_t m_Capacity = 8;
		unsafePoolAllocator_t m_Allocator;
	};

	template<typename T>
	inline Dynamic_Array<T>::Dynamic_Array()
		: m_Allocator(sizeof(T), m_Capacity, __alignof(T))
	{}

	template<typename T>
	inline Dynamic_Array<T>::~Dynamic_Array()
	{}

	template<typename T>
	inline T& Dynamic_Array<T>::operator[](const size_t a_Index) const
	{
		return *(reinterpret_cast<T*>(m_Allocator.begin()) + a_Index);
	}

	template<typename T>
	inline void Dynamic_Array<T>::push_back(T& a_Element)
	{
		T* t_AllocItem = reinterpret_cast<T*>(AllocNew<T>(m_Allocator, a_Element));
		m_Size++;
	}

	template<typename T>
	inline void Dynamic_Array<T>::resize(size_t a_Capacity)
	{

	}
}