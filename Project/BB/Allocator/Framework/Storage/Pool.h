#pragma once
#include "Utils/Logger.h"
#include "Allocators/AllocTypes.h"

namespace BB
{
	namespace Pool_Specs
	{
		constexpr const size_t multipleValue = 8;
		constexpr const size_t standardSize = 8;
	};

	//This class is outdated and needs updating.
	template<typename T>
	class Pool
	{
		static constexpr bool trivalDestructableT = std::is_trivially_destructible_v<T>;
		static constexpr bool trivialConstructableT = std::is_trivially_constructible_v<T>;

	public:
		Pool(Allocator a_Allocator);
		Pool(Allocator a_Allocator, const size_t a_Size);
		Pool(const Pool<T>& a_Pool) = delete; //no copy constructor, need to remake how the pool works since it doesn't make sense to have this.
		Pool(Pool<T>&& a_Pool) = delete; //no assignment constructor, need to remake how the pool works since it doesn't make sense to have this.
		~Pool();

		Pool<T>& operator=(const Pool<T>& a_Rhs) = delete; //no copy operator, need to remake how the pool works since it doesn't make sense to have this.
		Pool<T>& operator=(Pool<T>&& a_Rhs) = delete; //no assignment operator, need to remake how the pool works since it doesn't make sense to have this.

		T* Get();
		void Free(T* a_Ptr);

		const size_t size() const { return m_Size; };
		const void* data() const { return m_Start; };

	private:
		Allocator m_Allocator;

		size_t m_Size;
		T* m_Start;
		T** m_Pool;
	};

	template<typename T>
	inline BB::Pool<T>::Pool(Allocator a_Allocator)
		: Pool(a_Allocator, standardSize) {}

	template<typename T>
	inline BB::Pool<T>::Pool(Allocator a_Allocator, const size_t a_Size)
		: m_Allocator(a_Allocator), m_Size(a_Size)
	{
		BB_STATIC_ASSERT(sizeof(T) >= sizeof(void*), "Pool object is smaller then the size of a pointer.");
		BB_ASSERT(a_Size != 0, "Pool is created with an object size of 0!");

		m_Start = reinterpret_cast<T*>(BBalloc(m_Allocator, m_Size * sizeof(T)));
		m_Pool = reinterpret_cast<T**>(m_Start);

		T** t_Pool = m_Pool;

		for (size_t i = 0; i < m_Size - 1; i++)
		{
			*t_Pool = (reinterpret_cast<T*>(t_Pool)) + 1;
			t_Pool = reinterpret_cast<T**>(*t_Pool);
		}
		*t_Pool = nullptr;
	}

	template<typename T>
	inline Pool<T>::~Pool()
	{
		if (m_Start == nullptr)
		{
			if constexpr (!trivalDestructableT)
			{
				for (size_t i = 0; i < m_Size; i++)
				{
					m_Start[i].~T();
				}
			}
			BBfree(m_Allocator, m_Start);
		}
	}

	template<class T>
	inline T* Pool<T>::Get()
	{
		if (m_Pool == nullptr)
		{
			BB_WARNING(false, "Trying to get an pool object while there are none left!", WarningType::HIGH);
			return nullptr;
		}

		T* t_Ptr = reinterpret_cast<T*>(m_Pool);
		if constexpr (!trivialConstructableT)
		{
			new (t_Ptr) T();
		}
		m_Pool = reinterpret_cast<T**>(*m_Pool);

		return t_Ptr;
	}

	template<typename T>
	inline void BB::Pool<T>::Free(T* a_Ptr)
	{
		BB_ASSERT((a_Ptr >= m_Start && a_Ptr < m_Start + m_Size), "Trying to free an pool object that is not part of this pool!");
		(*reinterpret_cast<T**>(a_Ptr)) = reinterpret_cast<T*>(m_Pool);
		if constexpr (!trivalDestructableT)
		{
			a_Ptr->~T();
		}
		m_Pool = reinterpret_cast<T**>(a_Ptr);
	}
}