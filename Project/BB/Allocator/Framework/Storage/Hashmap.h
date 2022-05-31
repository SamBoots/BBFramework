#pragma once
constexpr uint32_t STANDARDHASHMAPSIZE = 1024;
#include "Framework/Allocators/BB_AllocTypes.h"
#include "Utils/Hash.h"

namespace BB
{
	constexpr uint8_t MAPEMPTY = 0x01;

#pragma region Unordered_Map
	//Unordered Map, uses linked list for collision.
	template<typename Value, typename Key, typename Allocator>
	class UM_HashMap
	{
		struct HashEntry
		{
			HashEntry* next_Entry;

			union
			{
				uint64_t state = MAPEMPTY;
				Key key;
			};
			Value value;
		};
		HashEntry* m_Entries;
		Allocator& m_Allocator;

	public:
		UM_HashMap(Allocator& a_Allocator);
		~UM_HashMap();

		void Insert(Value& a_Res, Key& a_Key);
		Value* Find(const Key& a_Key) const;
		void Remove(const Key& a_Key);

	private:
		bool Match(const HashEntry* a_Entry, const Key& a_Key) const;
	};

	template<typename Value, typename Key, typename Allocator>
	UM_HashMap<Value, Key, Allocator>::UM_HashMap(Allocator& a_Allocator)
		: m_Allocator(a_Allocator)
	{
		m_Entries = BBallocArray<HashEntry>(m_Allocator, STANDARDHASHMAPSIZE);
	}

	template<typename Value, typename Key, typename Allocator>
	UM_HashMap<Value, Key, Allocator>::~UM_HashMap()
	{
		//go through all the entries and individually delete the extra values from the linked list.
		//They need to be deleted seperatly since the memory is somewhere else.

		BBFreeArray(m_Allocator, m_Entries);
	}

	template<typename Value, typename Key, typename Allocator>
	void UM_HashMap<Value, Key, Allocator>::Insert(Value& a_Res, Key& a_Key)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash.hash = t_Hash % STANDARDHASHMAPSIZE;

		HashEntry* t_Entry = &m_Entries[t_Hash.hash];
		if (t_Entry->state == MAPEMPTY)
		{
			t_Entry->key = a_Key;
			t_Entry->value = a_Res;
			t_Entry->next_Entry = nullptr;
			return;
		}
		//Collision accurred, no problem we just create a linked list and make a new element.
		//Bad for cache memory though.
		while (t_Entry)
		{
			if (t_Entry->next_Entry == nullptr)
			{
				HashEntry* t_NewEntry = BBalloc<HashEntry, Allocator>(m_Allocator);
				t_NewEntry->key = a_Key;
				t_NewEntry->value = a_Res;
				t_NewEntry->next_Entry = nullptr;
				t_Entry->next_Entry = t_NewEntry;
				return;
			}
			t_Entry = t_Entry->next_Entry;
		}
	}

	template<typename Value, typename Key, typename Allocator>
	Value* UM_HashMap<Value, Key, Allocator>::Find(const Key& a_Key) const
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % STANDARDHASHMAPSIZE;

		HashEntry* t_Entry = &m_Entries[t_Hash];

		if (t_Entry->state == MAPEMPTY)
			return nullptr;

		while (t_Entry)
		{
			if (Match(t_Entry, a_Key))
			{
				return &t_Entry->value;
			}
			t_Entry = t_Entry->next_Entry;
		}
		return nullptr;
	}

	template<typename Value, typename Key, typename Allocator>
	inline void UM_HashMap<Value, Key, Allocator>::Remove(const Key& a_Key)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % STANDARDHASHMAPSIZE;


		HashEntry* t_Entry = &m_Entries[t_Hash];
		if (t_Entry->next_Entry == nullptr)
		{
			t_Entry->state = MAPEMPTY;
			t_Entry->value.~Value();
			return;
		}

		HashEntry* t_PreviousEntry = nullptr;

		while (t_Entry)
		{
			if (Match(t_Entry, a_Key))
			{
				t_PreviousEntry->next_Entry = t_Entry->next_Entry;
				BBFree(m_Allocator, t_Entry);
				return;
			}
			t_PreviousEntry = t_Entry;
			t_Entry = t_Entry->next_Entry;
		}
	}

	template<typename Value, typename Key, typename Allocator>
	bool UM_HashMap<Value, Key, Allocator>::Match(const HashEntry* a_Entry, const Key& a_Key) const
	{
		if (a_Entry->key == a_Key)
		{
			return true;
		}
		return false;
	}

#pragma endregion

#pragma region Open Addressing Linear (OL)
	//Open addressing with Linear probing.
	template<typename Value, typename Key, typename Allocator>
	class OL_HashMap
	{
		size_t m_Capacity;

		//All the elements.
		Hash* m_Hashes;
		Key* m_Keys;
		Value* m_Values;


		Allocator& m_Allocator;

	public:
		OL_HashMap(Allocator& a_Allocator);
		~OL_HashMap();

		void Insert(Value& a_Res, Key& a_Key);
		Value* Find(const Key& a_Key) const;
		void Remove(const Key& a_Key);
	};


	template<typename Value, typename Key, typename Allocator>
	inline OL_HashMap<Value, Key, Allocator>::OL_HashMap(Allocator& a_Allocator)
		:	m_Allocator(a_Allocator)
	{
		const size_t t_MemorySize = (sizeof(Hash) + sizeof(Key) + sizeof(Value)) * STANDARDHASHMAPSIZE;
		void* t_Buffer = BBalloc(m_Allocator, t_MemorySize);

		m_Capacity = STANDARDHASHMAPSIZE;

		m_Hashes = reinterpret_cast<Hash*>(t_Buffer);
		memset(m_Hashes, 0, sizeof(Hash) * STANDARDHASHMAPSIZE);
		m_Keys = reinterpret_cast<Key*>(pointerutils::Add(t_Buffer, sizeof(Hash) * STANDARDHASHMAPSIZE));
		m_Values = reinterpret_cast<Value*>(pointerutils::Add(t_Buffer, sizeof(Hash) * sizeof(Key) * STANDARDHASHMAPSIZE));
	}

	template<typename Value, typename Key, typename Allocator>
	inline OL_HashMap<Value, Key, Allocator>::~OL_HashMap()
	{
		BBFree(m_Allocator, m_Hashes);
	}

	template<typename Value, typename Key, typename Allocator>
	inline void OL_HashMap<Value, Key, Allocator>::Insert(Value& a_Res, Key& a_Key)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % STANDARDHASHMAPSIZE;

		while (m_Hashes[t_Hash] != 0)
		{
			t_Hash++;
		}
		m_Hashes[t_Hash] = t_Hash;
		m_Keys[t_Hash] = a_Key;
		m_Values[t_Hash] = a_Res;
	}

	template<typename Value, typename Key, typename Allocator>
	inline Value* OL_HashMap<Value, Key, Allocator>::Find(const Key& a_Key) const
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % STANDARDHASHMAPSIZE;

		while (m_Keys[t_Hash] != a_Key)
		{
			t_Hash++;
		}

		return &m_Values[t_Hash];
	}

	template<typename Value, typename Key, typename Allocator>
	inline void OL_HashMap<Value, Key, Allocator>::Remove(const Key& a_Key)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % STANDARDHASHMAPSIZE;

		while (m_Keys[t_Hash] != a_Key)
		{
			t_Hash++;
		}
		m_Hashes[t_Hash] = 0;
		m_Keys[t_Hash] = 0;
		m_Values[t_Hash].~Value();
	}

}

#pragma endregion