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

template<u32 CHUNK = 32>
class FixedChunkSize : public SparseSet
{
public:
	FixedChunkSize(res::Buffer& buffer, u32 bufferCapacity) 
		: m_bufferPtr{&buffer}
		, m_capacity{bufferCapacity}
	{}


public:
	// it is assumed that index is valid
	void mark(Handle index)
	{
		add(index / CHUNK);
	}

	void flush()
	{
		for (auto chunkNum : *this)
		{
			auto index = chunkNum * CHUNK;

			GLintptr offset = index * sizeof(IndirectDispatchBuffer);
			GLsizei size = std::min<u32>(CHUNK, m_capacity - index) * sizeof(IndirectDispatchBuffer);

			glFlushMappedNamedBufferRange(m_bufferPtr->id, offset, size);
		}
	}


private:
	res::Buffer* m_bufferPtr{nullptr};
	u32 m_capacity{};
};


// used in DirichletSystem along with data storage
// index comes from another storage
template<class FlushPolicy = FixedChunkSize<>>
class IndirectDispatchBufferStorage : protected FlushPolicy
{
public:
	IndirectDispatchBufferStorage(u32 bufferCapacity) : FlushPolicy(m_storage, bufferCapacity), m_capacity(bufferCapacity)
	{
		auto storageFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
		if (!try_create_storage_buffer(m_storage, bufferCapacity * sizeof(IndirectDispatchBuffer), storageFlags))
		{
			std::cerr << "Failed to create IndirectDispatchBufferStorage." << std::endl;

			assert(false);
		}

		m_mappedBufferPtr = (IndirectDispatchBuffer*)glMapNamedBuffer(m_storage.id, GL_READ_WRITE);
		if (m_mappedBufferPtr == nullptr)
		{
			std::cerr << "Failed to map storage." << std::endl;

			assert(false);
		}
	}

	IndirectDispatchBufferStorage(const IndirectDispatchBufferStorage&) = delete;

	IndirectDispatchBufferStorage(IndirectDispatchBufferStorage&& another) noexcept
		: m_storage{std::move(another.m_storage)}
		, m_capacity{std::exchange(another.m_capacity, 0u)}
		, m_mappedBufferPtr{std::exchange(another.m_mappedBufferPtr, nullptr)}
	{}

	~IndirectDispatchBufferStorage()
	{
		if (m_storage.valid() && m_mappedBufferPtr != nullptr)
		{
			glUnmapNamedBuffer(m_storage.id);

			m_mappedBufferPtr = nullptr;

			m_storage.reset();

			m_capacity = 0u;
		}
	}

	IndirectDispatchBufferStorage& operator = (const IndirectDispatchBufferStorage&) = delete;

	IndirectDispatchBufferStorage& operator = (IndirectDispatchBufferStorage&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		m_storage = std::move(another.m_storage);
		m_capacity = std::exchange(another.m_capacity, 0u);
		m_mappedBufferPtr = std::exchange(another.m_mappedBufferPtr, nullptr);

		return *this;
	}


public:
	IndirectDispatchBuffer& operator [] (Handle index)
	{
		FlushPolicy::mark(index);

		return m_mappedBufferPtr[index];
	}

	const IndirectDispatchBuffer& operator [] (Handle index) const
	{
		return m_mappedBufferPtr[index];
	}

	void flush()
	{
		FlushPolicy::flush();
	}


public:
	u32 capacity() const
	{
		return m_capacity;
	}


private:
	res::Buffer m_storage;
	u32 m_capacity{};

	IndirectDispatchBuffer* m_mappedBufferPtr{nullptr};
};
