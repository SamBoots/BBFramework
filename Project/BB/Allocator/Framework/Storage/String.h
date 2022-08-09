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
		String(Allocator a_Allocator, const char* a_String);
		String(Allocator a_Allocator, const char* a_String, size_t a_Size);
		String(const String& a_String);
		String(String&& a_String) noexcept;
		~String();

		String& operator=(const String& a_Rhs);
		String& operator=(String&& a_Rhs) noexcept;
		bool operator==(const String& a_Rhs) const;

		void append(const String& a_String);
		void append(const String& a_String, size_t a_SubPos, size_t a_SubLength);
		void append(const char* a_String);
		void append(const char* a_String, size_t a_Size);
		void insert(size_t a_Pos, const String& a_String);
		void insert(size_t a_Pos, const String& a_String, size_t a_SubPos, size_t a_SubLength);
		void insert(size_t a_Pos, const char* a_String);
		void insert(size_t a_Pos, const char* a_String, size_t a_Size);
		void push_back(const char a_Char);
		
		void pop_back();

		bool compare(const String& a_String) const;
		bool compare(const String& a_String, size_t a_Size) const;
		bool compare(size_t a_Pos, const String& a_String, size_t a_Subpos, size_t a_Size) const;
		bool compare(const char* a_String) const;
		bool compare(const char* a_String, size_t a_Size) const;
		bool compare(size_t a_Pos, const char* a_String) const;
		bool compare(size_t a_Pos, const char* a_String, size_t a_Size) const;

		void clear();

		void reserve(const size_t a_Size);
		void shrink_to_fit();

		size_t size() const { return m_Size; }
		size_t capacity() const { return m_Capacity; }
		void* data() const { return m_String; }
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
		m_Capacity = Math::RoundUp(a_Size, String_Specs::multipleValue);

		m_String = reinterpret_cast<char*>(BBalloc(m_Allocator, m_Capacity));
		memset(m_String, NULL, m_Capacity);
	}

	inline BB::String::String(Allocator a_Allocator, const char* a_String)
		:	String(a_Allocator, a_String, strlen(a_String))
	{}

	inline BB::String::String(Allocator a_Allocator, const char* a_String, size_t a_Size)
	{
		m_Allocator = a_Allocator;
		m_Capacity = Math::RoundUp(a_Size + 1, String_Specs::multipleValue);
		m_Size = a_Size;

		m_String = reinterpret_cast<char*>(BBalloc(m_Allocator, m_Capacity));
		BB::Memory::Copy(m_String, a_String, a_Size);
		memset(m_String + a_Size, NULL, m_Capacity - a_Size);
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

	inline bool BB::String::operator==(const String& a_Rhs) const
	{
		if (memcmp(m_String, a_Rhs.data(), m_Size) == 0)
			return true;
		return false;
	}

	inline void BB::String::append(const String& a_String)
	{
		append(a_String.c_str(), a_String.size());
	}

	inline void BB::String::append(const String& a_String, size_t a_SubPos, size_t a_SubLength)
	{
		append(a_String.c_str() + a_SubPos, a_SubLength);
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

	inline void BB::String::insert(size_t a_Pos, const String& a_String)
	{
		insert(a_Pos, a_String.c_str(), a_String.size());
	}

	inline void BB::String::insert(size_t a_Pos, const String& a_String, size_t a_SubPos, size_t a_SubLength)
	{
		insert(a_Pos, a_String.c_str() + a_SubPos, a_SubLength);
	}

	inline void BB::String::insert(size_t a_Pos, const char* a_String)
	{
		insert(a_Pos, a_String, strlen(a_String));
	}

	inline void BB::String::insert(size_t a_Pos, const char* a_String, size_t a_Size)
	{
		BB_ASSERT(m_Size >= a_Pos, "String::Insert, trying to insert a string in a invalid position.");

		if (m_Size + 1 + a_Size >= m_Capacity)
			grow(a_Size + 1);

		BB::Memory::Move(m_String + (a_Pos + a_Size), m_String + a_Pos, m_Size - a_Pos);

		BB::Memory::Copy(m_String + a_Pos, a_String, a_Size);
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

	inline bool BB::String::compare(const String& a_String) const
	{
		if (memcmp(m_String, a_String.data(), m_Size) == 0)
			return true;
		return false;
	}

	inline bool BB::String::compare(const String& a_String, size_t a_Size) const
	{
		if (memcmp(m_String, a_String.c_str(), a_Size) == 0)
			return true;
		return false;
	}

	inline bool BB::String::compare(size_t a_Pos, const String& a_String, size_t a_Subpos, size_t a_Size) const
	{
		if (memcmp(m_String + a_Pos, a_String.c_str() + a_Subpos, a_Size) == 0)
			return true;
		return false;
	}

	inline bool BB::String::compare(const char* a_String) const
	{
		return compare(a_String, strlen(a_String));
	}

	inline bool BB::String::compare(const char* a_String, size_t a_Size) const
	{
		if (memcmp(m_String, a_String, a_Size) == 0)
			return true;
		return false;
	}

	inline bool BB::String::compare(size_t a_Pos, const char* a_String) const
	{
		return compare(a_Pos, a_String, strlen(a_String));
	}

	inline bool BB::String::compare(size_t a_Pos, const char* a_String, size_t a_Size) const
	{
		if (memcmp(m_String + a_Pos, a_String, a_Size) == 0)
			return true;
		return false;
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