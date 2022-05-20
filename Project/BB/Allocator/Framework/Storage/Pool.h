#pragma once
#include "pch.h"
#include "Framework/BB_AllocTypes.h"

namespace BB
{
	template<typename T, typename Allocator>
	class Pool
	{
	public:
		Pool(const size_t a_Size, Allocator& a_Allocator);
		~Pool();

		T* Get();
		void Free(T* a_Ptr);

		const size_t size() const { return m_Size; };
		const void* data() const { return m_Allocator.begin(); };

	private:
		Allocator& m_Allocator;

		size_t m_Size;
		T* m_Start;
		T** m_Pool;
	};

	template<typename T, typename Allocator>
	inline BB::Pool<T, Allocator>::Pool(const size_t a_Size, Allocator& a_Allocator)
		: m_Size(a_Size), m_Allocator(a_Allocator)
	{
		BB_ASSERT(sizeof(T) >= sizeof(void*), "Pool object is smaller then the size of a pointer.");
		BB_ASSERT(a_Size != 0, "Pool is created with an object size of 0!");

		m_Pool = reinterpret_cast<T**>(BBallocArray<T>(m_Allocator, m_Size));
		m_Start = reinterpret_cast<T*>(m_Pool);

		T** t_Pool = m_Pool;

		for (size_t i = 0; i < m_Size - 1; i++)
		{
			*t_Pool = (reinterpret_cast<T*>(t_Pool)) + 1;
			t_Pool = reinterpret_cast<T**>(*t_Pool);
		}
		*t_Pool = nullptr;
	}

	template<typename T, typename Allocator>
	inline Pool<T, Allocator>::~Pool()
	{
		BBFreeArray<T>(m_Allocator, m_Start);
	}

	template<class T, typename Allocator>
	inline T* Pool<T, Allocator>::Get()
	{
		if (m_Pool == nullptr)
		{
			BB_WARNING(false, "Trying to get an pool object while there are none left!");
			return nullptr;
		}

		T* t_Ptr = reinterpret_cast<T*>(m_Pool);
		m_Pool = reinterpret_cast<T**>(*m_Pool);

		return t_Ptr;
	}

	template<typename T, typename Allocator>
	inline void BB::Pool<T, Allocator>::Free(T* a_Ptr)
	{
		BB_ASSERT((a_Ptr >= m_Start && a_Ptr < m_Start + m_Size), "Trying to free an pool object that is not part of this pool!");
		(*reinterpret_cast<T**>(a_Ptr)) = reinterpret_cast<T*>(m_Pool);
		m_Pool = reinterpret_cast<T**>(a_Ptr);
	}
}