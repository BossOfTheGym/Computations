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


	// folder containing all shaders
	ShaderStorage(const fs::path& shaderFolder);

public:
	void clear();

	/* json
		"shaders" : {
			...
			<shader_rel_path_i> : {
				<shader_config_i>
			},
			...
		}
	*/
	bool loadAll(const cfg::json& config);

	/* see shader-loader for json config spec */
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
