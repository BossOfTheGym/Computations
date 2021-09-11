#pragma once

#include <unordered_map>

#include <fs.h>
#include <cfg-fwd.h>

#include "shader-loader.h"
#include "shader-path-resolver.h"

class ShaderStorage
{
public:
	using Storage = std::unordered_map<std::string, gl::Shader>;
	using Iterator = typename Storage::const_iterator;
	using Status = std::pair<Iterator, bool>;

	ShaderStorage(const fs::path& shaderFolder);

public:
	void clear();

	bool loadAll(const cfg::json& config);

	Status load(const cfg::json& config, const fs::path& shaderPath);
	bool unload(const std::string& shaderName);

	Iterator find(const std::string& shaderName) const;
	bool has(const std::string& shaderName) const;

	Iterator begin() const;
	Iterator end() const;

private:
	ShaderLoader m_shaderLoader;
	ShaderPathResolver m_pathResolver;
	Storage m_shaders;	
};
