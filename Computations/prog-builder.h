#pragma once

#include <string>
#include <vector>

#include <gl-cxx/gl-res.h>

class ProgramBuilder
{
public:
	void reset();

	void attachShader(gl::Id id);
	void detachShader(gl::Id id);
	gl::ShaderProgram build();

private:
	std::vector<gl::Id> m_ids;
};
