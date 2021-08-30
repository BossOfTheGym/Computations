#pragma once

#include "app-system.h"
#include "element_registry.h"
#include <dirichlet/dirichlet-2d.h>

namespace app
{
	class App;


	class DirichletSystem : public System, public ElementRegistry<DirichletSystem>
	{
	public:
		DirichletSystem(app::App& app) : System(app)
		{}
	};
}
