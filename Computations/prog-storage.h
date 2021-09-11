#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <cfg-fwd.h>
#include <gl-cxx/gl-res.h>

#include "shader-provider.h"
#include "prog-builder.h"

class ProgramStorage
{
public:
	struct Entry
	{
		gl::ShaderProgram program;
		std::vector<std::string> shaders;
	};

	using Storage = std::unordered_map<std::string, Entry>;
	using Iterator = typename Storage::const_iterator;
	using Status = std::pair<Iterator, bool>;

public:
	void clear();

	bool loadAll(const cfg::json& config, IShaderProvider& shaderProvider);

	Status load(const std::string& name, const cfg::json& config, IShaderProvider& shaderProvider);
	bool unload(const std::string& name);

	Iterator find(const std::string& name);
	bool has(const std::string& name);

	Iterator begin() const;
	Iterator end() const;

private:
	ProgramBuilder m_builder;
	Storage m_programs;
};
