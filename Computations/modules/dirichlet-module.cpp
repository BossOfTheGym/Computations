#include "dirichlet-module.h"

#include <cfg.h>

ModulePtr DirichletModuleBuilder::build(Module& root, const cfg::json& config) override
{
	return {};
}

REGISTER_MODULE_BUILDER(dirichlet, DirichletModuleBuilder);