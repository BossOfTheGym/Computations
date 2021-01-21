#pragma once

#include "handle.h"

#include <vector>

template<class Type>
class Storage
{
	static_assert(std::is_move_assignable_v<Type> && std::is_default_constructible_v<Type>, "Type doesn't satisfy requirements.");

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

public:
	void add(Handle handle)
	{
		assert(handle != null);

		assure(handle);
		if (m_sparse[handle] == null)
		{
			m_sparse[handle] = m_packed.size();
			m_packed.push_back(handle);

			m_objects.emplace_back();
		}			
	}

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
		// TODO : aggregate and not aggregate
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
		return m_objects.begin();
	}

	auto begin() const
	{
		return m_objects.begin();
	}

	auto end()
	{
		return m_objects.end();
	}

	auto end() const
	{
		return m_objects.end();
	}


private:
	std::vector<Handle> m_sparse;
	std::vector<Handle> m_packed;

	std::vector<Type> m_objects;
};
