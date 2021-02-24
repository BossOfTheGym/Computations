#pragma once

#include "handle.h"

#include <vector>

class HandlePool
{
public:
	HandlePool() = default;

	HandlePool(HandlePool&& pool) noexcept
	{
		*this = std::move(pool);
	}

	HandlePool& operator = (HandlePool&& pool) noexcept
	{
		if (this == &pool)
		{
			return *this;
		}

		m_handles = std::move(pool.m_handles);
		m_head = std::exchange(pool.m_head, null);

		return *this;
	}
	
public:
	Handle acquire()
	{
		Handle newHandle{};
		if (m_head == null)
		{
			newHandle = Handle{m_handles.size()};
			m_handles.push_back(newHandle);
		}
		else
		{
			newHandle = m_head;
			m_head = m_handles[m_head];
			m_handles[newHandle] = newHandle;
		}
		return newHandle;
	}

	void release(Handle handle)
	{
		m_handles[handle] = m_head;
		m_head = handle;
	}

	bool valid(Handle handle) const
	{
		return handle < m_handles.size() && m_handles[handle] == handle;
	}


private:
	std::vector<Handle> m_handles;

	Handle m_head{null};
};
