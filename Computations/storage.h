#pragma once

#include "handle.h"

#include <utility>
#include <type_traits>
#include <vector>

// TODO : test iterator
template<class Type>
class Storage
{
	static_assert(std::is_move_assignable_v<Type> 
		&& std::is_move_constructible_v<Type> 
		&& std::is_default_constructible_v<Type>
		, "Type doesn't satisfy requirements.");

private:
	// non-standart iterator
	class Iterator
	{
		friend class Storage<Type>;

		using Handles = std::vector<Handle>;
		using Objects = std::vector<Type>;
		using Index = i32;

	private:
		Iterator(const Handles& handles, Objects& objects, Index index) 
			: m_handles{&handles}
			, m_objects{objects}
			, m_index{index}
		{}

	public:
		Iterator() = default;

	public:
		auto operator -> ()
		{
			return std::tie((*m_handles)[m_index], (*m_objects)[m_index]);
		}

		auto operator * ()
		{
			return operator -> ();
		}


		Iterator& operator ++ ()
		{
			return ++m_index, *this;
		}

		Iterator operator ++ (int)
		{
			auto orig = *this;

			return ++(*this), orig;
		}

		Iterator& operator -- ()
		{
			return --m_index, *this;
		}

		Iterator operator -- (int)
		{
			auto orig = *this;

			return --(*this), orig;
		}

		Iterator& operator += (Index value)
		{
			m_index += value;
			return *this;
		}

		Iterator operator + (Index value)
		{
			auto orig = *this;

			return (orig += value);
		}

		Iterator& operator -= (Index value)
		{
			m_index -= value;
			return *this;
		}

		Iterator operator - (Index value)
		{
			auto orig = *this;

			return (orig -= value);
		}

		Index operator - (const Iterator& another)
		{
			return m_index - another.m_index;
		}

		auto operator [] (Index value)
		{
			Index i = m_index + value;

			return std::tie((*m_handles)[i], (*m_objects)[i]);
		}

		bool operator == (const Iterator& another)
		{
			return m_index == another.m_index;
		}

		bool operator != (const Iterator& another)
		{
			return m_index != another.m_index;
		}

		bool operator < (const Iterator& another)
		{
			return m_index < another.m_index;
		}

		bool operator > (const Iterator& another)
		{
			return m_index > another.m_index;
		}

		bool operator <= (const Iterator& another)
		{
			return m_index <= another.m_index;
		}

		bool operator >= (const Iterator& another)
		{
			return m_index >= another.m_index;
		}


	private:
		const Handles* m_handles{nullptr};
		Objects* m_objects{nullptr};
		i32 m_index{-1};
	};


private:
	void assure(Handle handle)
	{
		if (handle >= m_sparse.size())
		{
			auto size = m_sparse.size();
			m_sparse.resize(handle + 1);
			for (; size < m_sparse.size(); size++)
			{
				m_sparse[size] = null;
			}
		}
	}

	void add(Handle handle)
	{
		assert(handle != null && !has(handle));

		assure(handle);
		if (m_sparse[handle] == null)
		{
			m_sparse[handle] = m_packed.size();
			m_packed.push_back(handle);
		}			
	}


public:
	bool has(Handle handle) const
	{
		return handle < m_sparse.size() && m_sparse[handle] != null;
	}

	void remove(Handle handle)
	{
		assert(has(handle));

		Handle index = m_sparse[handle];

		m_objects[index] = std::move(m_objects.back());
		m_objects.pop_back();

		m_packed[index] = m_packed.back();
		m_sparse[m_packed.back()] = index;
		m_sparse[handle] = null;
		m_packed.pop_back();
	}

	template<class ... Args>
	void emplace(Handle handle, Args&& ... args)
	{
		add(handle);

		if constexpr(std::is_aggregate_v<Type>)
		{
			m_objects.emplace_back(std::forward<Args>(args)...);
		}
		else
		{
			m_objects.push_back(Type(std::forward<Args>(args)...));
		}
	}

	Type& get(Handle handle)
	{
		assert(has(handle));

		return m_objects[m_sparse[handle]];
	}

	const Type& get(Handle handle) const
	{
		assert(has(handle));

		return m_objects[m_sparse[handle]];
	}

	Type& operator [] (Handle handle)
	{
		return get(handle);
	}

	const Type& operator [] (Handle handle) const
	{
		return get(handle);
	}


	// TODO : iterator
	auto begin()
	{
		return m_packed.begin();
	}

	auto begin() const
	{
		return m_packed.begin();
	}

	auto end()
	{
		return m_packed.end();
	}

	auto end() const
	{
		return m_packed.end();
	}


	i32 size() const
	{
		return m_packed.size();
	}


	Iterator pairsBegin()
	{
		return Iterator{m_packed, m_objects, 0};
	}

	Iterator pairsEnd()
	{
		return Iterator{m_packed, m_objects, typename Iterator::Index{m_packed.size()}};
	}

private:
	std::vector<Handle> m_sparse;
	std::vector<Handle> m_packed;

	std::vector<Type> m_objects;
};
