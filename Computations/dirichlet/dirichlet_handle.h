#pragma once

#include <utility>

#include <core.h>
#include <handle.h>

#include <gl-cxx/gl-fwd.h>

#include "dirichlet_fwd.h"

namespace dir2d
{
	// in fact IResourceProvider + handle wrapper
	// smart handle is either empty, or has valid handle (in perfect world)
	class SmartHandle
	{
		friend class SmartHandleProvider;

		SmartHandle(Handle handle, IResourceProvider* provider);

	public:
		SmartHandle() = default;

	public:
		void reset()
		{
			m_handle = null_handle;
			m_provider = nullptr;
		}

		bool empty() const
		{
			return m_handle == null_handle;
		}

		Handle handle() const
		{
			return m_handle;
		}


	public:
		bool valid() const;

		void destroy();

		gl::Id texture() const;

		const DomainAabb2D& domain() const;


	protected:
		Handle m_handle{null_handle};
		IResourceProvider* m_provider{};
	};

	// in fact IResourceProvider + handle wrapper
	// smart handle is either empty, or has valid handle (in perfect world)
	// scoped lifetime
	class ScopedHandle : public SmartHandle
	{
	public:
		~ScopedHandle()
		{
			if (!empty()) {
				destroy();
			}
		}

		ScopedHandle(SmartHandle&& handle) noexcept
		{
			static_cast<SmartHandle&>(*this) = std::move(handle);
		}

		ScopedHandle(ScopedHandle&& another) noexcept
		{
			static_cast<SmartHandle&>(*this) = std::move(another);

			another.reset();
		}

		ScopedHandle& operator = (ScopedHandle&& another) noexcept
		{
			return *this;
		}

		ScopedHandle(const ScopedHandle&) = delete;
		ScopedHandle& operator = (const ScopedHandle&) = delete;
	};


	// used to create smart handles
	class SmartHandleProvider
	{
	public:
		SmartHandle provideHandle(Handle handle, IResourceProvider* provider) const
		{
			return SmartHandle{handle, provider};
		}
	};
}
