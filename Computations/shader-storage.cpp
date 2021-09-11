#include "shader-storage.h"

ShaderStorage::ShaderStorage(const fs::path& shaderFolder)
	: m_pathResolver(shaderFolder)
{}

void ShaderStorage::clear()
{
	m_shaders.clear();
}

bool ShaderStorage::loadAll(const cfg::json& config)
{
	bool loadedSuccessfully = true;
	for (auto& [name, shaderConfig] : config) {
		auto [it, inserted] = load(shaderConfig, name);
		if (it == end() || !inserted) {
			loadedSuccessfully = false;
		}
	}
	return loadedSuccessfully;
}

auto ShaderStorage::load(const cfg::json& config, const fs::path& shaderPath) -> Status
{
	fs::path resolvedPath = m_pathResolver.resolve(shaderPath);
	gl::Shader loaded = m_shaderLoader.loadShader(config, resolvedPath);
	if (!loaded.valid()) {
		return {end(), false};
	}
	return m_shaders.insert({resolvedPath.string(), std::move(loaded)});
}

bool ShaderStorage::unload(const std::string& shaderName)
{
	return m_shaders.erase(shaderName) != 0;
}

auto ShaderStorage::find(const std::string& shaderName) -> Iterator const
{
	return m_shaders.find(shaderName);
}

bool ShaderStorage::has(const std::string& shaderName) const
{
	return find(shaderName) != end();
}

auto ShaderStorage::begin() const
{
	return m_shaders.begin();
}

auto ShaderStorage::end() const
{
	return m_shaders.end();
}
