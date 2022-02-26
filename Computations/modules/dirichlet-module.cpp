#include "dirichlet-module.h"
#include "module-dependencies.h"

#include "dirichlet-builder/dirichlet-builders.h"
#include "dirichlet-builder/dirichlet-builder-util.h"

ModulePtr DirichletModuleBuilder::build(Module& root, const cfg::json& config)
{
	auto modulePtr = std::make_shared<Module>();
	auto systemsPtr = std::make_shared<Module>();
	auto controlsPtr = std::make_shared<Module>();

	try_load_module(*modulePtr, systemsPtr, "systems");
	try_load_module(*modulePtr, controlsPtr, "controls");
	try_load_module(root, modulePtr, "dirichlet");

	auto& builders = ACCESS_DIRICHLET_BUILDERS();
	for (auto& [name, _] : config["dirichlet"].items()) {
		if (auto it = builders.find(name); it != builders.end()) {
			it->second->build(root, config);
		}
	}

	return modulePtr;
}

REGISTER_MODULE_BUILDER(dirichlet, DirichletModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(dirichlet, "program_storage");
