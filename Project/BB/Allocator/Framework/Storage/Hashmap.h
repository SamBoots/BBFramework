#pragma once
#include "Allocators/AllocTypes.h"
#include "Utils/Hash.h"

namespace BB
{
	namespace Hashmap_Specs
	{
		constexpr uint32_t Standard_Hashmap_Size = 64;

		constexpr const size_t multipleValue = 8;

		constexpr const float UM_LoadFactor = 1.f;
		constexpr const size_t Um_EmptyNode = 0x00;


		constexpr const float OL_LoadFactor = 1.3f;
		constexpr const uintptr_t OL_TOMBSTONE = 0xDEADBEEFDEADBEEF;
		constexpr const size_t OL_EMPTY = 0xAABBCCDD;
	};

	//Calculate the load factor.
	static size_t LFCalculation(size_t a_Size, float a_LoadFactor)
	{
		return static_cast<size_t>(static_cast<float>(a_Size) * (1.f / a_LoadFactor + 1));
	}

#pragma region Unordered_Map
	//Unordered Map, uses linked list for collision.
	template<typename Key, typename Value>
	class UM_HashMap
	{
		static constexpr bool trivalDestructableKey = std::is_trivially_destructible_v<Key>;
		static constexpr bool trivalDestructableValue = std::is_trivially_destructible_v<Value>;

		struct HashEntry
		{
			HashEntry* next_Entry;

			union
			{
				uint64_t state = Hashmap_Specs::Um_EmptyNode;
				Key key;
			};
			Value value;
		};

	public:
		UM_HashMap(Allocator a_Allocator);
		UM_HashMap(Allocator a_Allocator, const size_t a_Size);
		~UM_HashMap();

		void insert(Key& a_Key, Value& a_Res);
		template <class... Args>
		void emplace(Key& a_Key, Args&&... a_ValueArgs);
		Value* find(const Key& a_Key) const;
		void remove(const Key& a_Key);

		void reserve(const size_t a_Size);

	private:
		void reallocate(const size_t a_NewCapacity);

		size_t m_Capacity;
		size_t m_Size = 0;

		HashEntry* m_Entries;

		Allocator m_Allocator;

	private:
		bool Match(const HashEntry* a_Entry, const Key& a_Key) const
		{
			if (a_Entry->key == a_Key)
			{
				return true;
			}
			return false;
		}
	};

	template<typename Key, typename Value>
	inline UM_HashMap<Key, Value>::UM_HashMap(Allocator a_Allocator)
		: UM_HashMap(a_Allocator, Hashmap_Specs::Standard_Hashmap_Size)
	{}

	template<typename Key, typename Value>
	inline UM_HashMap<Key, Value>::UM_HashMap(Allocator a_Allocator, const size_t a_Size)
		: m_Allocator(a_Allocator)
	{
		m_Capacity = a_Size;
		m_Entries = reinterpret_cast<HashEntry*>(BBalloc(m_Allocator, m_Capacity * sizeof(HashEntry)));
	}

	template<typename Key, typename Value>
	inline UM_HashMap<Key, Value>::~UM_HashMap()
	{
		//go through all the entries and individually delete the extra values from the linked list.
		//They need to be deleted seperatly since the memory is somewhere else.
		for (size_t i = 0; i < m_Capacity; i++)
		{
			if (m_Entries[i].state != Hashmap_Specs::Um_EmptyNode)
			{
				HashEntry* t_NextEntry = m_Entries[i].next_Entry;
				while (t_NextEntry != nullptr)
				{
					HashEntry* t_DeleteEntry = t_NextEntry;
					t_NextEntry = t_NextEntry->next_Entry;
					//Call the destructor if it has one for the value.
					if constexpr (__has_user_destructor(Value))
						t_DeleteEntry->value.~Value();
					//Call the destructor if it has one for the key.
					if constexpr (__has_user_destructor(Key))
						t_DeleteEntry->key.~Key();

					BBfree(m_Allocator, t_DeleteEntry);
				}
			}
		}
		//Call the destructor if it has one for the value.
		if constexpr (!trivalDestructableValue)
			for (size_t i = 0; i < m_Capacity; i++)
				if (m_Entries[i].state == Hashmap_Specs::Um_EmptyNode)
					m_Entries[i].value.~Value();
		//Call the destructor if it has one for the key.
		if constexpr (!trivalDestructableKey)
			for (size_t i = 0; i < m_Capacity; i++)
				if (m_Entries[i].state == Hashmap_Specs::Um_EmptyNode)
					m_Entries[i].key.~Key();

		BBfree(m_Allocator, m_Entries);
	}

	template<typename Key, typename Value>
	inline void UM_HashMap<Key, Value>::insert(Key& a_Key, Value& a_Res)
	{
		emplace(a_Key, a_Res);
	}

	template<typename Key, typename Value>
	template <class... Args>
	inline void UM_HashMap<Key, Value>::emplace(Key& a_Key, Args&&... a_ValueArgs)
	{
		const Hash t_Hash = Hash::MakeHash(a_Key) % m_Capacity;

		HashEntry* t_Entry = &m_Entries[t_Hash.hash];
		if (t_Entry->state == Hashmap_Specs::Um_EmptyNode)
		{
			t_Entry->key = a_Key;
			new (&t_Entry->value) Value(std::forward<Args>(a_ValueArgs)...);
			t_Entry->next_Entry = nullptr;
			return;
		}
		//Collision accurred, no problem we just create a linked list and make a new element.
		//Bad for cache memory though.
		while (t_Entry)
		{
			if (t_Entry->next_Entry == nullptr)
			{
				HashEntry* t_NewEntry = BBalloc<HashEntry>(m_Allocator);
				t_NewEntry->key = a_Key;
				new (&t_NewEntry->value) Value(std::forward<Args>(a_ValueArgs)...);
				t_NewEntry->next_Entry = nullptr;
				t_Entry->next_Entry = t_NewEntry;
				return;
			}
			t_Entry = t_Entry->next_Entry;
		}
	}

	template<typename Key, typename Value>
	inline Value* UM_HashMap<Key, Value>::find(const Key& a_Key) const
	{
		const Hash t_Hash = Hash::MakeHash(a_Key) % m_Capacity;

		HashEntry* t_Entry = &m_Entries[t_Hash];

		if (t_Entry->state == Hashmap_Specs::Um_EmptyNode)
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

	template<typename Key, typename Value>
	inline void UM_HashMap<Key, Value>::remove(const Key& a_Key)
	{
		const Hash t_Hash = Hash::MakeHash(a_Key) % m_Capacity;;

		HashEntry* t_Entry = &m_Entries[t_Hash];
		if (Match(t_Entry, a_Key))
		{
			//Call the destructor if it has one for the value.
			if constexpr (!trivalDestructableValue)
				t_Entry->value.~Value();
			//Call the destructor if it has one for the key.
			if constexpr (!trivalDestructableKey)
				t_Entry->key.~Key();

			if (t_Entry->next_Entry != nullptr)
			{
				HashEntry* t_NextEntry = t_Entry->next_Entry;
				*t_Entry = *t_Entry->next_Entry;
				BBfree(m_Allocator, t_NextEntry);
				return;
			}

			t_Entry->state = Hashmap_Specs::Um_EmptyNode;
			return;
		}

		HashEntry* t_PreviousEntry = nullptr;

		while (t_Entry)
		{
			if (Match(t_Entry, a_Key))
			{
				t_PreviousEntry = t_Entry->next_Entry;
				BBfree(m_Allocator, t_Entry);
				return;
			}
			t_PreviousEntry = t_Entry;
			t_Entry = t_Entry->next_Entry;
		}
	}

	template<typename Key, typename Value>
	inline void UM_HashMap<Key, Value>::reserve(const size_t a_Size)
	{
		if (a_Size > m_Capacity)
		{
			size_t t_ModifiedCapacity = Math::RoundUp(a_Size * Hashmap_Specs::OL_LoadFactor, Hashmap_Specs::multipleValue);

			reallocate(t_ModifiedCapacity);
		}
	}

	template<typename Key, typename Value>
	inline void BB::UM_HashMap<Key, Value>::reallocate(const size_t a_NewCapacity)
	{
		//Allocate the new buffer.
		HashEntry* t_NewEntries = reinterpret_cast<HashEntry*>(BBalloc(m_Allocator, a_NewCapacity * sizeof(HashEntry)));

		for (size_t i = 0; i < m_Capacity; i++)
		{
			if (m_Entries[i].state != Hashmap_Specs::Um_EmptyNode)
			{
				const Hash t_Hash = Hash::MakeHash(m_Entries[i].key) % a_NewCapacity;

				HashEntry* t_Entry = &t_NewEntries[t_Hash.hash];
				if (t_Entry->state == Hashmap_Specs::Um_EmptyNode)
				{
					*t_Entry = m_Entries[i];

					return;
				}
				//Collision accurred, no problem we just create a linked list and make a new element.
				//Bad for cache memory though.
				while (t_Entry)
				{
					if (t_Entry->next_Entry == nullptr)
					{
						HashEntry* t_NewEntry = BBalloc<HashEntry>(m_Allocator);
						*t_NewEntry = m_Entries[i];
						return;
					}
					t_Entry = t_Entry->next_Entry;
				}
			}
		}

		this->~UM_HashMap();

		m_Capacity = a_NewCapacity;
		m_Entries = t_NewEntries;
	}

#pragma endregion

#pragma region Open Addressing Linear Probing (OL)
	//Open addressing with Linear probing.
	template<typename Key, typename Value>
	class OL_HashMap
	{
		static constexpr bool trivalDestructableValue = std::is_trivially_destructible_v<Value>;
		static constexpr bool trivalDestructableKey = std::is_trivially_destructible_v<Key>;

	public:
		OL_HashMap(Allocator a_Allocator);
		OL_HashMap(Allocator a_Allocator, const size_t a_Size);
		~OL_HashMap();

		void insert(Key& a_Key, Value& a_Res);
		template <class... Args>
		void emplace(Key& a_Key, Args&&... a_ValueArgs);
		Value* find(const Key& a_Key) const;
		void remove(const Key& a_Key);

		void reserve(const size_t a_Size);

	private:
		void grow(size_t a_MinCapacity = 1);
		void reallocate(const size_t a_NewLoadFactor);

	private:
		size_t m_Capacity;
		size_t m_Size;
		size_t m_LoadFactor;

		//All the elements.
		Hash* m_Hashes;
		Key* m_Keys;
		Value* m_Values;

		Allocator m_Allocator;
	};

	template<typename Key, typename Value>
	inline OL_HashMap<Key, Value>::OL_HashMap(Allocator a_Allocator)
		: OL_HashMap(a_Allocator, Hashmap_Specs::Standard_Hashmap_Size)
	{}

	template<typename Key, typename Value>
	inline OL_HashMap<Key, Value>::OL_HashMap(Allocator a_Allocator, const size_t a_Size)
		: m_Allocator(a_Allocator)
	{
		m_Capacity = LFCalculation(a_Size, Hashmap_Specs::OL_LoadFactor);
		m_Size = 0;
		m_LoadFactor = a_Size;

		const size_t t_MemorySize = (sizeof(Hash) + sizeof(Key) + sizeof(Value)) * m_Capacity;

		void* t_Buffer = BBalloc(m_Allocator, t_MemorySize);
		m_Hashes = reinterpret_cast<Hash*>(t_Buffer);
		m_Keys = reinterpret_cast<Key*>(pointerutils::Add(t_Buffer, sizeof(Hash) * m_Capacity));
		m_Values = reinterpret_cast<Value*>(pointerutils::Add(t_Buffer, (sizeof(Hash) + sizeof(Key)) * m_Capacity));
		std::fill(m_Hashes, m_Hashes + m_Capacity, Hashmap_Specs::OL_EMPTY);
	}

	template<typename Key, typename Value>
	inline OL_HashMap<Key, Value>::~OL_HashMap()
	{
		//Call the destructor if it has one for the value.
		if constexpr (!trivalDestructableValue)
			for (size_t i = 0; i < m_Capacity; i++)
				if (m_Hashes[i] != 0)
					m_Values[i].~Value();
		//Call the destructor if it has one for the key.
		if constexpr (!trivalDestructableKey)
			for (size_t i = 0; i < m_Capacity; i++)
				if (m_Hashes[i] != 0)
					m_Keys[i].~Key();

		BBfree(m_Allocator, m_Hashes);
	}

	template<typename Key, typename Value>
	inline void OL_HashMap<Key, Value>::insert(Key& a_Key, Value& a_Res)
	{
		emplace(a_Key, a_Res);
	}

	template<typename Key, typename Value>
	template <class... Args>
	inline void OL_HashMap<Key, Value>::emplace(Key& a_Key, Args&&... a_ValueArgs)
	{
		if (m_Size > m_LoadFactor)
			grow();

		m_Size++;
		const Hash t_Hash = Hash::MakeHash(a_Key) % m_Capacity;

		for (size_t i = t_Hash; i < m_Capacity; i++)
		{
			if (m_Hashes[i] == Hashmap_Specs::OL_EMPTY)
			{
				m_Hashes[i] = t_Hash;
				m_Keys[i] = a_Key;
				new (&m_Values[i]) Value(std::forward<Args>(a_ValueArgs)...);
				return;
			}
		}

		//Loop again but then from the start and stop at the hash. 
		for (size_t i = 0; i < t_Hash; i++)
		{
			if (m_Hashes[i] == Hashmap_Specs::OL_EMPTY)
			{
				m_Hashes[i] = t_Hash;
				m_Keys[i] = a_Key;
				new (&m_Values[i]) Value(std::forward<Args>(a_ValueArgs)...);
				return;
			}
		}
	}

	template<typename Key, typename Value>
	inline Value* OL_HashMap<Key, Value>::find(const Key& a_Key) const
	{
		const Hash t_Hash = Hash::MakeHash(a_Key) % m_Capacity;

		for (size_t i = t_Hash; i < m_Capacity; i++)
		{
			if (m_Keys[i] == a_Key)
				return &m_Values[i];
		}

		//Loop again but then from the start and stop at the hash. 
		for (size_t i = 0; i < t_Hash; i++)
		{
			if (m_Keys[i] == a_Key)
				return &m_Values[i];
		}

		//Key does not exist.
		return nullptr;
	}

	template<typename Key, typename Value>
	inline void OL_HashMap<Key, Value>::remove(const Key& a_Key)
	{
		const Hash t_Hash = Hash::MakeHash(a_Key) % m_Capacity;

		for (size_t i = t_Hash; i < m_Capacity; i++)
		{
			if (m_Keys[i] == a_Key)
			{
				m_Hashes[i] = Hashmap_Specs::OL_EMPTY;
				//Call the destructor if it has one for the value.
				if constexpr (!trivalDestructableValue)
					m_Values[i].~Value();
				//Call the destructor if it has one for the key.
				if constexpr (!trivalDestructableKey)
					m_Keys[i].~Key();
				m_Keys[i] = 0;

				m_Size--;
				return;
			}
		}

		//Loop again but then from the start and stop at the hash. 
		for (size_t i = 0; i < t_Hash; i++)
		{
			if (m_Keys[i] == a_Key)
			{
				m_Hashes[i] = Hashmap_Specs::OL_EMPTY;
				//Call the destructor if it has one for the value.
				if constexpr (!trivalDestructableValue)
					m_Values[i].~Value();
				//Call the destructor if it has one for the key.
				if constexpr (!trivalDestructableKey)
					m_Keys[i].~Key();
				m_Keys[i] = 0;

				m_Size--;
				return;
			}
		}
		BB_EXCEPTION(false, "OL_Hashmap remove called but key not found!");
	}

	template<typename Key, typename Value>
	inline void OL_HashMap<Key, Value>::reserve(const size_t a_Size)
	{
		if (a_Size > m_Capacity)
		{
			size_t t_ModifiedCapacity = Math::RoundUp(a_Size, Hashmap_Specs::multipleValue);

			reallocate(t_ModifiedCapacity);
		}
	}

	template<typename Key, typename Value>
	inline void OL_HashMap<Key, Value>::grow(size_t a_MinCapacity)
	{
		BB_WARNING(false, "Resizing an OL_HashMap, this might be a bit slow. Possibly reserve more.");

		size_t t_ModifiedCapacity = m_Capacity * 2;

		if (a_MinCapacity > t_ModifiedCapacity)
			t_ModifiedCapacity = Math::RoundUp(a_MinCapacity, Dynamic_Array_Specs::multipleValue);

		reallocate(t_ModifiedCapacity);
	}

	template<typename Key, typename Value>
	inline void OL_HashMap<Key, Value>::reallocate(const size_t a_NewLoadFactor)
	{
		const size_t t_NewCapacity = LFCalculation(a_NewLoadFactor, Hashmap_Specs::OL_LoadFactor);

		//Allocate the new buffer.
		const size_t t_MemorySize = (sizeof(Hash) + sizeof(Key) + sizeof(Value)) * t_NewCapacity;
		void* t_Buffer = BBalloc(m_Allocator, t_MemorySize);

		Hash* t_NewHashes = reinterpret_cast<Hash*>(t_Buffer);
		Key* t_NewKeys = reinterpret_cast<Key*>(pointerutils::Add(t_Buffer, sizeof(Hash) * t_NewCapacity));
		Value* t_NewValues = reinterpret_cast<Value*>(pointerutils::Add(t_Buffer, (sizeof(Hash) + sizeof(Key)) * t_NewCapacity));
		std::fill(t_NewHashes, t_NewHashes + t_NewCapacity, Hashmap_Specs::OL_EMPTY);

		for (size_t i = 0; i < m_Capacity; i++)
		{
			if (m_Hashes[i] != 0)
			{
				Key t_Key = m_Keys[i];
				Hash t_Hash = Hash::MakeHash(t_Key) % t_NewCapacity;

				while (t_NewHashes[t_Hash] != Hashmap_Specs::OL_EMPTY)
				{
					t_Hash++;
					if (t_Hash > t_NewCapacity)
						t_Hash = 0;
				}
				t_NewHashes[t_Hash] = t_Hash;
				t_NewKeys[t_Hash] = t_Key;
				t_NewValues[t_Hash] = m_Values[i];
			}
		}

		//Remove all the elements and free the memory.
		this->~OL_HashMap();

		m_Hashes = t_NewHashes;
		m_Keys = t_NewKeys;
		m_Values = t_NewValues;

		m_Capacity = t_NewCapacity;
		m_LoadFactor = a_NewLoadFactor;
	}
}

#pragma endregion