#pragma once

#include <fs.h>
#include <cfg-fwd.h>
#include <gl-cxx/gl-res.h>

#include <string>

#include "glsl-preproc.h"

class ShaderLoader
{
public:
	// gets json config as a parameter
	// see shader config specification
	// path should be relative to shader folder stored in config
	gl::Shader loadShader(const cfg::json& config, const fs::path& path);

private:
	bool configureShaderSource(const cfg::json& config, const fs::path& path, std::string& shaderSource);

private:
	GlslPreprocessor m_preprocessor;
};
