#pragma once

#include <fs.h>
#include <string>

class ShaderPathResolver
{
public:	
	ShaderPathResolver(const fs::path& shaderFolder);

public:
	// result can be empty which mmeans that shaderPath is not relative to shaderFolder
	fs::path resolve(const fs::path& shaderPath);

	fs::path shaderFolder() const
	{
		return m_shaderFolder;
	}

private:
	fs::path m_shaderFolder;
};
