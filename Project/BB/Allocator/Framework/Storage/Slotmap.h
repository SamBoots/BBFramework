#pragma once
#include "Allocators/AllocTypes.h"
#include "Utils/Utils.h"

namespace BB
{
	namespace Slotmap_Specs
	{

	}

	struct SlotmapID
	{
		size_t index; //Can also have the index to the next free id.
	};

	template <typename T>
	class Slotmap
	{
	public:
		struct Node
		{
			SlotmapID id;
			T value;
		};

		struct Iterator
		{
			Iterator(Node* a_Ptr) : m_Ptr(a_Ptr) {}

			T& operator*() const { return m_Ptr->value; }
			T* operator->() { return &m_Ptr->value; }

			Iterator& operator++()
			{
				m_Ptr++;
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator t_Tmp = *this;
				++(*this);
				return t_Tmp;
			}

			friend bool operator< (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr < a_Rhs.m_Ptr; };
			friend bool operator> (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr > a_Rhs.m_Ptr; };
			friend bool operator<= (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr <= a_Rhs.m_Ptr; };
			friend bool operator>= (const Iterator& a_Lhs, const Iterator& a_Rhs) { return a_Lhs.m_Ptr >= a_Rhs.m_Ptr; };

		private:
			Node* m_Ptr;
		};

		Slotmap();
		~Slotmap();

		SlotmapID insert(T& a_Obj);
		T& find(SlotmapID a_ID);
		void erase(SlotmapID a_ID);

		Iterator begin() { return Iterator(m_ObjArr); }
		Iterator end() { return Iterator(&m_ObjArr[m_Capacity]); }

	private:
		size_t m_Capacity = 128;
		size_t m_Size = 0;
		size_t m_NextFree;

		SlotmapID m_IdArr[128];
		Node m_ObjArr[128];
	};

	template<typename T>
	inline BB::Slotmap<T>::Slotmap()
	{
		for (size_t i = 0; i < m_Capacity; ++i)
		{
			m_IdArr[i].index = i + 1;
		}
		m_NextFree = 0;
	}

	template<typename T>
	inline BB::Slotmap<T>::~Slotmap()
	{

	}

	template<typename T>
	inline SlotmapID BB::Slotmap<T>::insert(T& a_Obj)
	{
		SlotmapID& t_Id = m_IdArr[m_NextFree];
		m_NextFree = t_Id.index;

		t_Id.index = m_Size++;

		Node& t_Node = m_ObjArr[t_Id.index];
		t_Node.id = t_Id;
		t_Node.value = a_Obj;

		return t_Id;
	}

	template<typename T>
	inline T& BB::Slotmap<T>::find(SlotmapID a_ID)
	{
		return m_ObjArr[a_ID.index].value;
	}

	template<typename T>
	inline void BB::Slotmap<T>::erase(SlotmapID a_ID)
	{
		size_t t_OldFree = m_NextFree;
		m_NextFree = a_ID.index;
		m_IdArr[a_ID.index].index = t_OldFree;

		Slotmap::Node& t_Node = m_ObjArr[--m_Size];
		t_Node.id.index = t_OldFree;
		t_Node.value = std::move(m_ObjArr[a_ID.index].value);
	}
}