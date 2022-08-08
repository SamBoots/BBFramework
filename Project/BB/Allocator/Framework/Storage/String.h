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

	class String
	{
	public:
		String(Allocator a_Allocator);
		String(Allocator a_Allocator, size_t a_Size);
		String(const String& a_String);
		String(String&& a_String) noexcept;
		~String();

		String& operator=(const String& a_Rhs);
		String& operator=(String&& a_Rhs) noexcept;

		void append(const char* a_String);
		void append(const char* a_String, size_t a_Size);
		void push_back(const char a_Char);
		
		void pop_back();

		void clear();

		void reserve(const size_t a_Size);
		void shrink_to_fit();

		size_t size() const { return m_Size; }
		size_t capacity() const { return m_Capacity; }
		const char* data() const { return m_String; }
		const char* c_str() const { return m_String; }

	private:
		void grow(size_t a_MinCapacity = 1);
		void reallocate(size_t a_NewCapacity);

		Allocator m_Allocator;

		char* m_String;
		size_t m_Size = 0;
		size_t m_Capacity = 64;
	};

	inline BB::String::String(Allocator a_Allocator)
		: String(a_Allocator, String_Specs::standardSize)
	{}

	inline BB::String::String(Allocator a_Allocator, size_t a_Size)
	{
		m_Allocator = a_Allocator;
		m_Capacity = a_Size;

		m_String = reinterpret_cast<char*>(BBalloc(m_Allocator, m_Capacity));
		memset(m_String, NULL, m_Capacity);
	}

	inline BB::String::String(const String& a_String)
	{
		m_Allocator = a_String.m_Allocator;
		m_Capacity = a_String.m_Capacity;
		m_Size = a_String.m_Size;

		m_String = reinterpret_cast<char*>(BBalloc(m_Allocator, m_Capacity));
		memcpy(m_String, a_String.m_String, m_Capacity);
	}

	inline BB::String::String(String&& a_String) noexcept
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

	inline BB::String::~String()
	{
		if (m_String != nullptr)
		{
			BBfree(m_Allocator, m_String);
		}
	}

	inline String& BB::String::operator=(const String& a_Rhs)
	{
		this->~String();

		m_Allocator = a_Rhs.m_Allocator;
		m_Capacity = a_Rhs.m_Capacity;
		m_Size = a_Rhs.m_Size;

		m_String = reinterpret_cast<char*>(BBalloc(m_Allocator, m_Capacity));
		memcpy(m_String, a_Rhs.m_String, m_Capacity);

		return *this;
	}

	inline String& BB::String::operator=(String&& a_Rhs) noexcept
	{
		this->~String();

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

	inline void BB::String::append(const char* a_String)
	{
		append(a_String, strlen(a_String));
	}

	inline void BB::String::append(const char* a_String, size_t a_Size)
	{
		if (m_Size + 1 + a_Size >= m_Capacity)
			grow(a_Size + 1);

		BB::Memory::Copy(m_String + m_Size, a_String, a_Size);
		m_Size += a_Size;
	}

	inline void BB::String::push_back(const char a_Char)
	{
		if (m_Size + 1 >= m_Capacity)
			grow();

		m_String[m_Size++] = a_Char;
	}

	inline void BB::String::pop_back()
	{
		m_String[m_Size--] = NULL;
	}

	inline void BB::String::clear()
	{
		memset(m_String, NULL, m_Size);
		m_Size = 0;
	}

	inline void BB::String::reserve(const size_t a_Size)
	{
		if (a_Size > m_Capacity)
		{
			size_t t_ModifiedCapacity = Math::RoundUp(a_Size + 1, String_Specs::multipleValue);

			reallocate(t_ModifiedCapacity);
		}
	}

	inline void BB::String::shrink_to_fit()
	{
		size_t t_ModifiedCapacity = Math::RoundUp(m_Size + 1, String_Specs::multipleValue);
		if (t_ModifiedCapacity < m_Capacity)
		{
			reallocate(t_ModifiedCapacity);
		}
	}

	inline void BB::String::grow(size_t a_MinCapacity)
	{
		size_t t_ModifiedCapacity = m_Capacity * 2;

		if (a_MinCapacity > t_ModifiedCapacity)
			t_ModifiedCapacity = Math::RoundUp(a_MinCapacity, Array_Specs::multipleValue);

		reallocate(t_ModifiedCapacity);
	}

	inline void BB::String::reallocate(size_t a_NewCapacity)
	{
		char* t_NewString = reinterpret_cast<char*>(BBalloc(m_Allocator, a_NewCapacity * sizeof(char)));

		Memory::Copy(t_NewString, m_String, m_Size);
		BBfree(m_Allocator, m_String);

		m_String = t_NewString;
		m_Capacity = a_NewCapacity;
	}
}