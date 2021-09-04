#pragma once

#include "handle.h"

#include <vector>
#include <utility>
#include <cassert>
#include <type_traits>


class SparseSet
{
protected:
	void assure(Handle handle)
	{
		if (handle >= m_sparse.size())
		{
			auto size = m_sparse.size();
			m_sparse.resize(handle + 1);
			for (; size < m_sparse.size(); size++)
			{
				m_sparse[size] = null_handle;
			}
		}
	}

	Handle index(Handle handle) const
	{
		return m_sparse[handle];
	}

	void add(Handle handle)
	{
		assure(handle);
		if (m_sparse[handle] == null_handle)
		{
			m_sparse[handle] = m_packed.size();
			m_packed.push_back(handle);
		}			
	}

	void remove(Handle handle)
	{
		m_packed[m_sparse[handle]] = m_packed.back();
		m_sparse[m_packed.back()] = m_sparse[handle];

		m_sparse[handle] = null_handle;
		m_packed.pop_back();
	}

	bool has(Handle handle) const
	{
		return handle < m_sparse.size() && m_sparse[handle] != null_handle;
	}


public:
	auto begin()
	{
		return m_packed.begin();
	}

	auto end()
	{
		return m_packed.end();
	}

	auto begin() const
	{
		return m_packed.begin();
	}

	auto end() const
	{
		return m_packed.end();
	}

	auto size() const
	{
		return m_packed.size();
	}


private:
	std::vector<Handle> m_sparse;
	std::vector<Handle> m_packed;
};

// TODO : iterator
// TODO : type check
template<class Type>
class Storage : public SparseSet
{
public:
	void remove(Handle handle)
	{
		assert(has(handle));

		Handle i = index(handle);

		m_objects[i] = std::move(m_objects.back());
		m_objects.pop_back();

		SparseSet::remove(handle);
	}

	template<class ... Args>
	void emplace(Handle handle, Args&& ... args)
	{
		assert(!has(handle));

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

		return m_objects[index(handle)];
	}

	const Type& get(Handle handle) const
	{
		assert(has(handle));

		return m_objects[index(handle)];
	}

	bool has(Handle handle) const
	{
		return SparseSet::has(handle);
	}

	Type& operator [] (Handle handle)
	{
		return get(handle);
	}

	const Type& operator [] (Handle handle) const
	{
		return get(handle);
	}


private:
	std::vector<Type> m_objects;
};
