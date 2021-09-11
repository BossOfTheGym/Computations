#include "shader-path-resolver.h"

#include <exception>

ShaderPathResolver::ShaderPathResolver(const fs::path& shaderFolder)
	: m_shaderFolder(fs::absolute(shaderFolder))
{}

fs::path ShaderPathResolver::resolve(const fs::path& shaderPath)
{
	return fs::relative(fs::absolute(shaderPath), m_shaderFolder);
}
