#include "program-builder.h"

#include <algorithm>

#include <gl-cxx/gl-res-util.h>

void ProgramBuilder::reset()
{
	m_ids.clear();
}

void ProgramBuilder::attachShader(gl::Id id)
{
	auto it = std::find(m_ids.begin(), m_ids.end(), id);
	if (it == m_ids.end()) {
		m_ids.push_back(id);
	}
}

void ProgramBuilder::detachShader(gl::Id id)
{
	auto it = std::remove(m_ids.begin(), m_ids.end(), id);
	m_ids.erase(it, m_ids.end());
}

gl::ShaderProgram ProgramBuilder::build()
{
	if (m_ids.empty()) {
		return {};
	}
	return gl::create_shader_program(m_ids.size(), m_ids.data());
}