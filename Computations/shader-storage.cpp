#include "shader-storage.h"

#include <cfg.h>

#include <iostream>

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
	for (auto& [name, shaderConfig] : config.items()) {
		auto [it, inserted] = load(shaderConfig, name);

		// TODO : workaround, remove (engineering niggas)
		if (it == end() || !inserted) {
			loadedSuccessfully = false;
			std::cout << "Shader " << name << " loaded successfully." << std::endl;
		}
		else {
			std::cout << "Failed to load shader " << name << " ." << std::endl;
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

auto ShaderStorage::find(const std::string& shaderName) const -> Iterator
{
	return m_shaders.find(shaderName);
}

bool ShaderStorage::has(const std::string& shaderName) const
{
	return find(shaderName) != end();
}

auto ShaderStorage::begin() const -> Iterator
{
	return m_shaders.begin();
}

auto ShaderStorage::end() const -> Iterator
{
	return m_shaders.end();
}
