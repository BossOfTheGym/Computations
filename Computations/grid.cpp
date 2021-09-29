#include "grid.h"

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include <stdexcept>

Grid::Grid(uint gridX, uint gridY, uint width, uint height, gl::Id program)
	: m_gridX{gridX}
	, m_gridY{gridY}
	, m_width{width}
	, m_height{height}
	, m_program{program}
{
	if (m_program == gl::null) {
		throw std::runtime_error("Grid: program is null.");
	}

	m_dummy = gl::create_vertex_array();
	if (!m_dummy.valid()) {
		throw std::runtime_error("Grid: Failed to create vretex array.");
	}
}

void Grid::setup()
{
	glUseProgram(m_program);
	glBindVertexArray(m_dummy.id);
}

void Grid::render(const dir2d::SmartHandle& handle, uint index)
{
	if (index >= m_gridX * m_gridY) {
		return;
	}

	uint i = index % m_gridY;
	uint j = index / m_gridX;
	uint w = m_width / m_gridX;
	uint h = m_height / m_gridY;

	constexpr int UNIT_INDEX = 0;

	glViewport(j * w, i * h, w, h);
	glBindTextureUnit(UNIT_INDEX, handle.texture());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
