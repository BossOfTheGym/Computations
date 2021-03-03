#pragma once

#include "handle.h"

#include <cassert>
#include <vector>

class HandlePool
{
public:
	HandlePool() = default;

	HandlePool(HandlePool&& pool) noexcept
		: m_handles(std::move(pool.m_handles))
		, m_head{std::exchange(pool.m_head, null)}
	{}

	~HandlePool()
	{
		m_handles.clear();

		m_head = null;
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
		assert(valid(handle));

		m_handles[handle] = m_head;
		m_head = handle;
	}

	bool valid(Handle handle) const
	{
		return handle < m_handles.size() && m_handles[handle] == handle;
	}

	bool full() const
	{
		return m_head == null;
	}


private:
	std::vector<Handle> m_handles;

	Handle m_head{null};
};

class FixedHandlePool
{
public:
	FixedHandlePool(u32 poolSize = 0u) : m_size{poolSize}
	{
		if (poolSize != 0u)
		{
			m_handles = std::make_unique<Handle[]>(poolSize);

			m_handles[0] = null;
			for (u32 i = 1; i < poolSize; i++)
			{
				m_handles[i] = i - 1;
			}
			m_head = m_handles[poolSize - 1];
		}
	}

	FixedHandlePool(FixedHandlePool&& another) noexcept
		: m_handles(std::move(another.m_handles))
		, m_head{std::exchange(another.m_head, null)}
		, m_size{std::exchange(another.m_size, 0)}
	{}

	~FixedHandlePool()
	{
		m_size = 0;

		m_handles.reset();

		m_head = null;
	}

	FixedHandlePool& operator = (FixedHandlePool&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		m_size = std::exchange(another.m_size, 0);
		m_head = std::exchange(another.m_head, null);
		m_handles = std::move(another.m_handles);

		return *this;
	}


public:
	Handle acquire()
	{
		assert(!full());

		Handle acquired{m_head};
		m_head = m_handles[m_head];
		m_handles[acquired] = acquired;

		return acquired;
	}

	void release(Handle handle)
	{
		assert(valid(handle));

		m_handles[handle] = m_head;
		m_head = handle;
	}

	bool valid(Handle handle) const
	{
		return handle < m_size && m_handles[handle] == handle;
	}

	bool full() const
	{
		return m_head == null;
	}


private:
	u32 m_size{};

	std::unique_ptr<Handle[]> m_handles;

	Handle m_head{null};
};
