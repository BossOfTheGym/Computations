#pragma once

#include <core.h>
#include <gl-cxx/gl-fwd.h>
#include <gl-cxx/gl-res.h>
#include <dirichlet/dirichlet_handle.h>

class Grid
{
public:
	Grid(uint gridX, uint gridY, uint width, uint height, gl::Id program);

	void setup();

	void render(const dir2d::SmartHandle& handle, uint index);

private:
	uint m_gridX{};
	uint m_gridY{};
	uint m_width{};
	uint m_height{};
	gl::Id m_program{};
	gl::VertexArray m_dummy;
};
