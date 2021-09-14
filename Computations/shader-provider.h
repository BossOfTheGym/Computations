#pragma once

#include <string>

#include <gl-cxx/gl-fwd.h>

class IShaderProvider
{
public:
	virtual ~IShaderProvider() = default;

	virtual gl::Id get(const std::string& name) const = 0;
};