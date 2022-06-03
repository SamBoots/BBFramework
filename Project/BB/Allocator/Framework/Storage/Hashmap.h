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

	private:
		void grow(size_t a_MinumumSize = 1);
	};


	template<typename Value, typename Key, typename Allocator>
	inline OL_HashMap<Value, Key, Allocator>::OL_HashMap(Allocator& a_Allocator)
		:	m_Allocator(a_Allocator)
	{
		m_Capacity = STANDARDHASHMAPSIZE;

		const size_t t_MemorySize = (sizeof(Hash) + sizeof(Key) + sizeof(Value)) * m_Capacity;

		void* t_Buffer = BBalloc(m_Allocator, t_MemorySize);

		m_Hashes = reinterpret_cast<Hash*>(t_Buffer);
		memset(m_Hashes, 0, (sizeof(Hash) + sizeof(Key)) * m_Capacity);
		m_Keys = reinterpret_cast<Key*>(pointerutils::Add(t_Buffer, sizeof(Hash) * m_Capacity));
		m_Values = reinterpret_cast<Value*>(pointerutils::Add(t_Buffer, (sizeof(Hash) + sizeof(Key)) * m_Capacity));
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
		t_Hash = t_Hash % m_Capacity;

		for (size_t i = t_Hash; i < m_Capacity; i++)
		{
			if (m_Hashes[i] == 0)
			{
				m_Hashes[i] = t_Hash;
				m_Keys[i] = a_Key;
				m_Values[i] = a_Res;
				return;
			}
		}

		//Loop again but then from the start and stop at the hash. 
		for (size_t i = 0; i < t_Hash; i++)
		{
			if (m_Hashes[i] == 0)
			{
				m_Hashes[i] = t_Hash;
				m_Keys[i] = a_Key;
				m_Values[i] = a_Res;
				return;
			}
		}

		grow();
		Insert(a_Res, a_Key);
	}

	template<typename Value, typename Key, typename Allocator>
	inline Value* OL_HashMap<Value, Key, Allocator>::Find(const Key& a_Key) const
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % m_Capacity;
		Hash t_AdjustedHash = m_Hashes[t_Hash];

		for (size_t i = t_AdjustedHash; i < m_Capacity; i++)
		{
			if (m_Keys[m_Hashes[i]] == a_Key)
				return &m_Values[m_Hashes[i]];
		}

		//Loop again but then from the start and stop at the hash. 
		for (size_t i = 0; i < t_AdjustedHash; i++)
		{
			if (m_Keys[m_Hashes[i]] == a_Key)
				return &m_Values[m_Hashes[i]];
		}

		//Key does not exist.
		return nullptr;
	}

	template<typename Value, typename Key, typename Allocator>
	inline void OL_HashMap<Value, Key, Allocator>::Remove(const Key& a_Key)
	{
		Hash t_Hash = Hash::MakeHash(a_Key);
		t_Hash = t_Hash % m_Capacity;
		Hash t_AdjustedHash = m_Hashes[t_Hash];

		for (size_t i = t_AdjustedHash; i < m_Capacity; i++)
		{
			if (m_Keys[m_Hashes[i]] == a_Key)
			{
				m_Keys[m_Hashes[i]] = 0;
				m_Values[m_Hashes[i]].~Value();
				m_Hashes[m_Hashes[i]] = 0;
				return;
			}
		}

		//Loop again but then from the start and stop at the hash. 
		for (size_t i = 0; i < t_AdjustedHash; i++)
		{
			if (m_Keys[m_Hashes[i]] == a_Key)
			{
				m_Keys[m_Hashes[i]] = 0;
				m_Values[m_Hashes[i]].~Value();
				m_Hashes[m_Hashes[i]] = 0;
				return;
			}
		}
	}

	template<typename Value, typename Key, typename Allocator>
	inline void OL_HashMap<Value, Key, Allocator>::grow(size_t a_MinumumSize)
	{
		BB_WARNING(false, "Resizing an OL_HashMap, this might be a bit slow. Possibly reserve more.");

		size_t t_NewCapacity = m_Capacity * 2;

		//Allocate the new buffer.
		const size_t t_MemorySize = (sizeof(Hash) + sizeof(Key) + sizeof(Value)) * t_NewCapacity;
		void* t_Buffer = BBalloc(m_Allocator, t_MemorySize);

		Hash* t_NewHashes = reinterpret_cast<Hash*>(t_Buffer);
		Key* t_NewKeys = reinterpret_cast<Key*>(pointerutils::Add(t_Buffer, sizeof(Hash) * t_NewCapacity));
		Value* t_NewValues = reinterpret_cast<Value*>(pointerutils::Add(t_Buffer, (sizeof(Hash) + sizeof(Key)) * t_NewCapacity));
		memset(t_NewHashes, 0, (sizeof(Hash) + sizeof(Key)) * m_Capacity);

		for (size_t i = 0; i < m_Capacity; i++)
		{
			Hash t_Hash = Hash::MakeHash(m_Keys[i]);
			Hash t_NewHash = t_Hash % t_NewCapacity;
			Hash t_OldHash = t_Hash % m_Capacity;

			t_NewHashes[t_NewHash] = m_Hashes[t_OldHash];
		}

		//Place the previous hashes back together with the keys and values.
		memcpy(t_NewKeys, m_Keys, m_Capacity * sizeof(Key));
		memcpy(t_NewValues, m_Values, m_Capacity * sizeof(Value));

		BBFree(m_Allocator, m_Hashes);
		m_Hashes = t_NewHashes;
		m_Keys = t_NewKeys;
		m_Values = t_NewValues;

		m_Capacity = t_NewCapacity;
	}
}

#pragma endregion