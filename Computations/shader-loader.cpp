#include "shader-loader.h"

#include <fs.h>
#include <cfg.h>
#include <file-util.h>
#include <gl-cxx/gl-res-util.h>

#include <fstream>

gl::Shader ShaderLoader::loadShader(const cfg::json& config, const fs::path& path)
{
	m_preprocessor.reset();

	std::string shaderSource;
	if (!configureShaderSource(config, path, shaderSource)) {
		return gl::Shader();
	}
	
	GLenum shaderType = gl::shader_type_from_extension(path);
	return gl::create_shader_from_source(shaderType, shaderSource);
}

bool ShaderLoader::configureShaderSource(const cfg::json& config, const fs::path& path, std::string& shaderSource)
{
	if (!m_preprocessor.addShaderSource(path.string())) {
		return false;
	}

	if (config.contains("version-string")) {
		auto& versionString = config["version-string"];
		if (versionString.is_string()) {
			m_preprocessor.specifyVersionString(versionString.get<std::string>());
		}
		else {
			return false;
		}
	}

	if (config.contains("macros")) {
		for (auto& [macro, value] : config["macros"].items()) {
			if (value.is_null()) {
				m_preprocessor.defineMacro(macro);
			}
			else if (value.is_string()) {
				m_preprocessor.addMacro(macro, value.get<std::string>());
			}
			else {
				return false;
			}
		}
	}
	return m_preprocessor.process(shaderSource);
}
