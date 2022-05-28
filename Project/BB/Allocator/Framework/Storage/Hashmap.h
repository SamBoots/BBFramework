#pragma once
constexpr uint32_t STANDARDHASHMAPSIZE = 1024;
#include "Framework/Allocators/BB_AllocTypes.h"
#include "Utils/Hash.h"

namespace BB
{
	template<typename Key, typename Value, typename Allocator>
	class HashMap
	{
		struct HashEntry
		{
			HashEntry* next_Entry = nullptr;

			Key key;
			Value value;
		};
		HashEntry* m_Entries;
		Allocator& m_Allocator;

	public:
		HashMap(Allocator& a_Allocator);
		~HashMap();

		void Insert(Key& a_Key, Value& a_Res);
		Value Find(Key& a_Key);

	private:
		bool Match(HashEntry* a_Entry, Key& a_Key);
	};

	template<typename Key, typename Value, typename Allocator>
	HashMap<Key, Value, Allocator>::HashMap(Allocator& a_Allocator)
		: m_Allocator(a_Allocator)
	{
		m_Entries = BBallocArray<HashEntry>(m_Allocator, STANDARDHASHMAPSIZE);
	}

	template<typename Key, typename Value, typename Allocator>
	HashMap<Key, Value, Allocator>::~HashMap()
	{

	}

	template<typename Key, typename Value, typename Allocator>
	void HashMap<Key, Value, Allocator>::Insert(Key& a_Key, Value& a_Res)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash.hash = t_Hash % STANDARDHASHMAPSIZE;

		m_HashEntry<Key, Value>* entry = BBalloc<HashEntry<Key, Value>>(m_Allocator);
		entry->key = a_Key;
		entry->value = a_Res;

		entry->next_Entry = m_Map[t_Hash];
		m_Map[t_Hash.hash] = entry;
	}

	template<typename Key, typename Value, typename Allocator>
	Value HashMap<Key, Value, Allocator>::Find(Key& a_Key)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % STANDARDHASHMAPSIZE;

		m_HashEntry<Key, Value>* entry = m_Map[t_Hash];
		while (entry)
		{
			if (Match(entry, a_Key))
			{
				return entry->value;
			}
			entry = entry->next_Entry;
		}
		return nullptr;
	}

	template<typename Key, typename Value, typename Allocator>
	bool HashMap<Key, Value, Allocator>::Match(HashEntry* a_Entry, Key& a_Key)
	{
		if (a_Entry->key == a_Key)
		{
			return true;
		}
		return false;
	}
}