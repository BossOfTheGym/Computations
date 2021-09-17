#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <cfg-fwd.h>
#include <gl-cxx/gl-res.h>

#include "shader-provider.h"
#include "program-builder.h"

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

	// json : {
	//		...
	// 	    <program_name_i> : [<shader_name>, ...],
	// 		...
	// }
	// shaderProvider : object that can access shader storage
	bool loadAll(const cfg::json& config, IShaderProvider& shaderProvider);

	// json : [<shader_name_i>, ...]
	Status load(const std::string& name, const cfg::json& config, IShaderProvider& shaderProvider);
	bool unload(const std::string& name);

	Iterator find(const std::string& name) const;
	bool has(const std::string& name) const;

	Iterator begin() const;
	Iterator end() const;

private:
	ProgramBuilder m_builder;
	Storage m_storage;
};
