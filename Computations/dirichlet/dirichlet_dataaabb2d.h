#pragma once

#include <core.h>

#include <memory>

#include "dirichlet_fwd.h"
#include "dirichlet_function.h"
#include "dirichlet_domainaabb2d.h"

namespace dir2d
{
	// Data accosiated with a given problem
	// div(grad(u)) = f
	// u(boundary) = g
	// first coord is y(rows), second coord is x(cols) as everything is stored in row-major manner
	// texture is 'padded' with boundary conditions
	// stores f32 data for single-channel texture
	struct DataAabb2D
	{
		static DataAabb2D create_data(const DomainAabb2D& domain, const Function2D& boundary, const Function2D& f);

		std::unique_ptr<f32[]> solution;
		std::unique_ptr<f32[]> f;
	};
}
