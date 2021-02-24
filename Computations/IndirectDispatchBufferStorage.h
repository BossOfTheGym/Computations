#pragma once

#include <cassert>
#include <iostream>

#include "core.h"
#include "handle.h"
#include "handle-pool.h"
#include "storage.h"
#include "graphics-res-util.h"


struct IndirectDispatchBuffer
{
	u32 numWorkgroupsX{};
	u32 numWorkgroupsY{};
	u32 numWorkgroupsZ{};
};

// TODO : checks
class IndirectDispatchBufferStorage : public HandlePool
{
public:
	IndirectDispatchBufferStorage(u32 storageSize)
	{
		storageSize -= storageSize % sizeof(IndirectDispatchBuffer);

		if (!try_create_storage_buffer(m_storage, storageSize, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT))
		{
			// TODO : decide what to do? maybe i should just throw an exception...
			//	... or just set state to invalid
			std::cerr << "Failed to create IndirectDispatchBufferStorage." << std::endl;
		}

		m_mappedBufferPtr = (IndirectDispatchBuffer*)glMapNamedBuffer(m_storage.id, GL_READ_WRITE);
		if (m_mappedBufferPtr == nullptr)
		{
			// TODO : decide what to to, maybe an exception will help
			std::cerr << "Failed to map storage." << std::endl;
		}
	}

	IndirectDispatchBufferStorage(const IndirectDispatchBufferStorage&) = delete;

	IndirectDispatchBufferStorage(IndirectDispatchBufferStorage&& storage) noexcept
	{
		*this = std::move(storage);
	}

	~IndirectDispatchBufferStorage()
	{
		reset();
	}

	IndirectDispatchBufferStorage& operator = (const IndirectDispatchBufferStorage&) = delete;

	IndirectDispatchBufferStorage& operator = (IndirectDispatchBufferStorage&& storage) noexcept
	{
		if (this == &storage)
		{
			return *this;
		}

		static_cast<HandlePool&>(*this) = std::move(storage);

		m_storage = std::move(storage.m_storage);
		m_mappedBufferPtr = std::exchange(storage.m_mappedBufferPtr, nullptr);

		return *this;
	}


public:
	bool storageValid() const
	{
		return m_storage.valid() && m_mappedBufferPtr != nullptr;
	}

	void reset()
	{
		if (storageValid())
		{
			glUnmapNamedBuffer(m_storage.id);

			m_mappedBufferPtr = nullptr;
		}
	}


public:
	IndirectDispatchBuffer& operator [] (Handle handle)
	{
		assert(storageValid() && valid(handle));

		return m_mappedBufferPtr[handle];
	}

	const IndirectDispatchBuffer& operator [] (Handle handle) const
	{
		assert(storageValid() && valid(handle));

		return m_mappedBufferPtr[handle];
	}


private:
	res::Buffer m_storage;

	IndirectDispatchBuffer* m_mappedBufferPtr{nullptr};
};
