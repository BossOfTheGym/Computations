#include "prog-storage.h"

#include <cfg.h>

void ProgramStorage::clear()
{
	m_storage.clear();
}

bool ProgramStorage::loadAll(const cfg::json& config, IShaderProvider& shaderProvider)
{
	bool loadedSuccessfully = true;
	for (auto& [name, programConfig]) {
		auto [it, inserted] = load(name, programConfig, shaderProvider);
		if (it == end() || !inserted) {
			loadedSuccessfully = false;
		}
	}
	return loadedSuccessfully;
}

auto ProgramStorage::load(const std::string& name, const cfg::json& config, IShaderProvider& shaderProvider) -> Status
{
	m_builder.clear();
	
	auto shaders = config.get<std::vector<std::string>>();
	for (auto& shader : entry.shaders) {
		gl::Id id = shaderProvider.get(shader);
		if (id == gl::null) {
			return {end(), false};
		}
		m_builder.attachShader(id);
	}

	auto program = m_builder.build();
	if (!program.valid()) {
		return {end(), false};
	}

	return m_programs.insert({std::move(shaders), std::move(program)});
}

bool ProgramStorage::unload(const std::string& name)
{
	return m_programs.erase(name) != 0;
}

auto ProgramStorage::find(const std::string& name) const -> Iterator
{
	return m_storage.find(name);
}

bool ProgramStorage::has(const std::string& name) const
{
	return find(name) != end();
}

auto ProgramStorage::begin() const -> Iterator
{
	return m_storage.begin();
}

auto ProgramStorage::end() const -> Iterator
{
	return m_storage.end();
}