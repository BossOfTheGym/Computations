#include "dirichlet_handle.h"
#include "resource_provider.h"

#include <cassert>

namespace dir2d
{
	SmartHandle::SmartHandle(Handle handle, IResourceProvider* provider)
		: m_handle{handle}
		, m_provider{provider}
	{
		assert(handle != null_handle);
		assert(provider != nullptr);
	}

	bool SmartHandle::valid() const
	{
		return m_provider->valid(m_handle);
	}

	void SmartHandle::destroy()
	{
		m_provider->destroy(m_handle);

		reset();
	}

	gl::Id SmartHandle::texture() const
	{
		return m_provider->texture(m_handle);
	}

	const DomainAabb2D& SmartHandle::domain() const
	{
		return m_provider->domain(m_handle);
	}
}
