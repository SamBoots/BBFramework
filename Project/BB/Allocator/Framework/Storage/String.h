#pragma once
#include "Utils/Logger.h"
#include "Allocators/AllocTypes.h"

#include "Utils/Utils.h"

namespace BB
{
	namespace String_Specs
	{
		constexpr const size_t multipleValue = 8;
		constexpr const size_t standardSize = 8;
	}

	template<typename T>
	class Basic_String
	{
	public:
		Basic_String(Allocator a_Allocator);
		Basic_String(Allocator a_Allocator, size_t a_Size);
		Basic_String(Allocator a_Allocator, const T* a_String);
		Basic_String(Allocator a_Allocator, const T* a_String, size_t a_Size);
		Basic_String(const Basic_String<T>& a_String);
		Basic_String(Basic_String<T>&& a_String) noexcept;
		~Basic_String();

		Basic_String& operator=(const Basic_String<T>& a_Rhs);
		Basic_String& operator=(Basic_String<T>&& a_Rhs) noexcept;
		bool operator==(const Basic_String<T>& a_Rhs) const;

		void append(const Basic_String<T>& a_String);
		void append(const Basic_String<T>& a_String, size_t a_SubPos, size_t a_SubLength);
		void append(const T* a_String);
		void append(const T* a_String, size_t a_Size);
		void insert(size_t a_Pos, const Basic_String<T>& a_String);
		void insert(size_t a_Pos, const Basic_String<T>& a_String, size_t a_SubPos, size_t a_SubLength);
		void insert(size_t a_Pos, const T* a_String);
		void insert(size_t a_Pos, const T* a_String, size_t a_Size);
		void push_back(const T a_Char);
		
		void pop_back();

		bool compare(const Basic_String<T>& a_String) const;
		bool compare(const Basic_String<T>& a_String, size_t a_Size) const;
		bool compare(size_t a_Pos, const Basic_String<T>& a_String, size_t a_Subpos, size_t a_Size) const;
		bool compare(const T* a_String) const;
		bool compare(const T* a_String, size_t a_Size) const;
		bool compare(size_t a_Pos, const T* a_String) const;
		bool compare(size_t a_Pos, const T* a_String, size_t a_Size) const;

		void clear();

		void reserve(const size_t a_Size);
		void shrink_to_fit();

		size_t size() const { return m_Size; }
		size_t capacity() const { return m_Capacity; }
		void* data() const { return m_String; }
		const T* c_str() const { return m_String; }

	private:
		void grow(size_t a_MinCapacity = 1);
		void reallocate(size_t a_NewCapacity);

		Allocator m_Allocator;

		T* m_String;
		size_t m_Size = 0;
		size_t m_Capacity = 64;
	};

	using String = Basic_String<char>;
	using WString = Basic_String<wchar_t>;


	template<typename T>
	inline BB::Basic_String<T>::Basic_String(Allocator a_Allocator)
		: Basic_String(a_Allocator, String_Specs::standardSize)
	{}

	template<typename T>
	inline BB::Basic_String<T>::Basic_String(Allocator a_Allocator, size_t a_Size)
	{
		constexpr bool is_char = std::is_same_v<T, char> || std::is_same_v<T, wchar_t>;
		BB_STATIC_ASSERT(is_char, "String is not a char or wchar");

		m_Allocator = a_Allocator;
		m_Capacity = Math::RoundUp(a_Size, String_Specs::multipleValue);

		m_String = reinterpret_cast<T*>(BBalloc(m_Allocator, m_Capacity * sizeof(T)));
		Memory::Set(m_String, NULL, m_Capacity);
	}

	template<typename T>
	inline BB::Basic_String<T>::Basic_String(Allocator a_Allocator, const T* a_String)
		:	Basic_String(a_Allocator, a_String, Memory::StrLength(a_String))
	{}

	template<typename T>
	inline BB::Basic_String<T>::Basic_String(Allocator a_Allocator, const T* a_String, size_t a_Size)
	{
		constexpr bool is_char = std::is_same_v<T, char> || std::is_same_v<T, wchar_t>;
		BB_STATIC_ASSERT(is_char, "String is not a char or wchar");

		m_Allocator = a_Allocator;
		m_Capacity = Math::RoundUp(a_Size + 1, String_Specs::multipleValue);
		m_Size = a_Size;

		m_String = reinterpret_cast<T*>(BBalloc(m_Allocator, m_Capacity * sizeof(T)));
		Memory::Copy(m_String, a_String, a_Size);
		Memory::Set(m_String + a_Size, NULL, m_Capacity - a_Size);
	}

	template<typename T>
	inline BB::Basic_String<T>::Basic_String(const Basic_String<T>& a_String)
	{
		m_Allocator = a_String.m_Allocator;
		m_Capacity = a_String.m_Capacity;
		m_Size = a_String.m_Size;

		m_String = reinterpret_cast<T*>(BBalloc(m_Allocator, m_Capacity * sizeof(T)));
		Memory::Copy(m_String, a_String.m_String, m_Capacity);
	}

	template<typename T>
	inline BB::Basic_String<T>::Basic_String(Basic_String<T>&& a_String) noexcept
	{
		m_Allocator = a_String.m_Allocator;
		m_Capacity = a_String.m_Capacity;
		m_Size = a_String.m_Size;
		m_String = a_String.m_String;

		a_String.m_Allocator.allocator = nullptr;
		a_String.m_Allocator.func = nullptr;
		a_String.m_Capacity = 0;
		a_String.m_Size = 0;
		a_String.m_String = nullptr;
	}

	template<typename T>
	inline BB::Basic_String<T>::~Basic_String()
	{
		if (m_String != nullptr)
		{
			BBfree(m_Allocator, m_String);
		}
	}

	template<typename T>
	inline Basic_String<T>& BB::Basic_String<T>::operator=(const Basic_String<T>& a_Rhs)
	{
		this->~Basic_String();

		m_Allocator = a_Rhs.m_Allocator;
		m_Capacity = a_Rhs.m_Capacity;
		m_Size = a_Rhs.m_Size;

		m_String = reinterpret_cast<T*>(BBalloc(m_Allocator, m_Capacity * sizeof(T)));
		Memory::Copy(m_String, a_Rhs.m_String, m_Capacity);

		return *this;
	}

	template<typename T>
	inline Basic_String<T>& BB::Basic_String<T>::operator=(Basic_String<T>&& a_Rhs) noexcept
	{
		this->~Basic_String();

		m_Allocator = a_Rhs.m_Allocator;
		m_Capacity = a_Rhs.m_Capacity;
		m_Size = a_Rhs.m_Size;
		m_String = a_Rhs.m_String;

		a_Rhs.m_Allocator.allocator = nullptr;
		a_Rhs.m_Allocator.func = nullptr;
		a_Rhs.m_Capacity = 0;
		a_Rhs.m_Size = 0;
		a_Rhs.m_String = nullptr;

		return *this;
	}

	template<typename T>
	inline bool BB::Basic_String<T>::operator==(const Basic_String<T>& a_Rhs) const
	{
		if (BB::Memory::Compare(m_String, a_Rhs.data(), m_Size) == 0)
			return true;
		return false;
	}

	template<typename T>
	inline void BB::Basic_String<T>::append(const Basic_String<T>& a_String)
	{
		append(a_String.c_str(), a_String.size());
	}

	template<typename T>
	inline void BB::Basic_String<T>::append(const Basic_String<T>& a_String, size_t a_SubPos, size_t a_SubLength)
	{
		append(a_String.c_str() + a_SubPos, a_SubLength);
	}

	template<typename T>
	inline void BB::Basic_String<T>::append(const T* a_String)
	{
		append(a_String, Memory::StrLength(a_String));
	}

	template<typename T>
	inline void BB::Basic_String<T>::append(const T* a_String, size_t a_Size)
	{
		if (m_Size + 1 + a_Size >= m_Capacity)
			grow(a_Size + 1);

		BB::Memory::Copy(m_String + m_Size, a_String, a_Size);
		m_Size += a_Size;
	}

	template<typename T>
	inline void BB::Basic_String<T>::insert(size_t a_Pos, const Basic_String<T>& a_String)
	{
		insert(a_Pos, a_String.c_str(), a_String.size());
	}

	template<typename T>
	inline void BB::Basic_String<T>::insert(size_t a_Pos, const Basic_String<T>& a_String, size_t a_SubPos, size_t a_SubLength)
	{
		insert(a_Pos, a_String.c_str() + a_SubPos, a_SubLength);
	}

	template<typename T>
	inline void BB::Basic_String<T>::insert(size_t a_Pos, const T* a_String)
	{
		insert(a_Pos, a_String, Memory::StrLength(a_String));
	}

	template<typename T>
	inline void BB::Basic_String<T>::insert(size_t a_Pos, const T* a_String, size_t a_Size)
	{
		BB_ASSERT(m_Size >= a_Pos, "String::Insert, trying to insert a string in a invalid position.");

		if (m_Size + 1 + a_Size >= m_Capacity)
			grow(a_Size + 1);

		BB::Memory::Move(m_String + (a_Pos + a_Size), m_String + a_Pos, m_Size - a_Pos);

		BB::Memory::Copy(m_String + a_Pos, a_String, a_Size);
		m_Size += a_Size;
	}

	template<typename T>
	inline void BB::Basic_String<T>::push_back(const T a_Char)
	{
		if (m_Size + 1 >= m_Capacity)
			grow();

		m_String[m_Size++] = a_Char;
	}

	template<typename T>
	inline void BB::Basic_String<T>::pop_back()
	{
		m_String[m_Size--] = NULL;
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(const Basic_String<T>& a_String) const
	{
		if (Memory::Compare(m_String, a_String.data(), m_Size) == 0)
			return true;
		return false;
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(const Basic_String<T>& a_String, size_t a_Size) const
	{
		if (Memory::Compare(m_String, a_String.c_str(), a_Size) == 0)
			return true;
		return false;
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(size_t a_Pos, const Basic_String<T>& a_String, size_t a_Subpos, size_t a_Size) const
	{
		if (Memory::Compare(m_String + a_Pos, a_String.c_str() + a_Subpos, a_Size) == 0)
			return true;
		return false;
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(const T* a_String) const
	{
		return compare(a_String, Memory::StrLength(a_String));
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(const T* a_String, size_t a_Size) const
	{
		if (Memory::Compare(m_String, a_String, a_Size) == 0)
			return true;
		return false;
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(size_t a_Pos, const T* a_String) const
	{
		return compare(a_Pos, a_String, Memory::StrLength(a_String));
	}

	template<typename T>
	inline bool BB::Basic_String<T>::compare(size_t a_Pos, const T* a_String, size_t a_Size) const
	{
		if (Memory::Compare(m_String + a_Pos, a_String, a_Size) == 0)
			return true;
		return false;
	}

	template<typename T>
	inline void BB::Basic_String<T>::clear()
	{
		Memory::Set(m_String, NULL, m_Size);
		m_Size = 0;
	}

	template<typename T>
	inline void BB::Basic_String<T>::reserve(const size_t a_Size)
	{
		if (a_Size > m_Capacity)
		{
			size_t t_ModifiedCapacity = Math::RoundUp(a_Size + 1, String_Specs::multipleValue);

			reallocate(t_ModifiedCapacity);
		}
	}

	template<typename T>
	inline void BB::Basic_String<T>::shrink_to_fit()
	{
		size_t t_ModifiedCapacity = Math::RoundUp(m_Size + 1, String_Specs::multipleValue);
		if (t_ModifiedCapacity < m_Capacity)
		{
			reallocate(t_ModifiedCapacity);
		}
	}

	template<typename T>
	inline void BB::Basic_String<T>::grow(size_t a_MinCapacity)
	{
		size_t t_ModifiedCapacity = m_Capacity * 2;

		if (a_MinCapacity > t_ModifiedCapacity)
			t_ModifiedCapacity = Math::RoundUp(a_MinCapacity, Array_Specs::multipleValue);

		reallocate(t_ModifiedCapacity);
	}

	template<typename T>
	inline void BB::Basic_String<T>::reallocate(size_t a_NewCapacity)
	{
		T* t_NewString = reinterpret_cast<T*>(BBalloc(m_Allocator, a_NewCapacity * sizeof(T)));

		Memory::Copy(t_NewString, m_String, m_Size);
		BBfree(m_Allocator, m_String);

		m_String = t_NewString;
		m_Capacity = a_NewCapacity;
	}
}