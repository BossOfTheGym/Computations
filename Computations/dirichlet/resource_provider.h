#pragma once

#include <handle.h>
#include <gl-cxx/gl-fwd.h>

#include "dirichlet_fwd.h"

namespace dir2d
{
	// provider something with resources : texture id & domain description.
	// interface of every dirichlet system
	// gives ability to check if handle is valid (in perfect world)
	class IResourceProvider
	{
	public:
		virtual ~IResourceProvider() = default;

	public:
		virtual bool valid(Handle handle) const = 0;
		virtual void destroy(Handle handle) = 0;

		virtual gl::Id texture(Handle handle) const = 0;
		virtual const DomainAabb2D& domain(Handle handle) const = 0;
	};
}