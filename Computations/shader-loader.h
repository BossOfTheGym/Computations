#pragma once

#include <fs.h>
#include <cfg-fwd.h>
#include <gl-cxx/gl-res.h>

#include <string>

#include "glsl-preproc.h"

class ShaderLoader
{
public:
	/* json :  {
			...
			<macro_name_i> : <macro_value_i>,
			...
		}
		path : path relative to shader folder
	*/
	gl::Shader loadShader(const cfg::json& config, const fs::path& path);

private:
	bool configureShaderSource(const cfg::json& config, const fs::path& path, std::string& shaderSource);

private:
	GlslPreprocessor m_preprocessor;
};
