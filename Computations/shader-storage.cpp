#include "shader-storage.h"

#include <cfg.h>
#include <gl-cxx/gl-res-util.h>

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
	auto test = fs::current_path();

	bool allLoaded = true;
	for (auto& entry: fs::recursive_directory_iterator(m_pathResolver.shaderFolder())) {
		auto path = entry.path();

		if (!entry.is_regular_file() || !gl::is_shader_file(path)) {
			continue;
		}

		auto [it, inserted] = load(config, path);
		if (it == end() || !inserted) {
			allLoaded = false;
			std::cout << "Failed to load shader " << path << " ." << std::endl;
		}
		else {
			std::cout << "Shader " << path << " loaded successfully." << std::endl;
		}
	}
	return allLoaded;
}

auto ShaderStorage::load(const cfg::json& config, const fs::path& shaderPath) -> Status
{
	std::string resolvedPathStr = m_pathResolver.resolve(shaderPath).string();
	if (resolvedPathStr.empty()) {
		return {end(), false};
	}

	if (!config.contains(resolvedPathStr)) {
		return {end(), false};
	}

	gl::Shader loaded = m_shaderLoader.loadShader(config[resolvedPathStr], shaderPath);
	if (!loaded.valid()) {
		return {end(), false};
	}

	return m_shaders.insert({std::move(resolvedPathStr), std::move(loaded)});
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
