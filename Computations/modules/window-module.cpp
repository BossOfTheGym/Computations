#include "window-module.h"

#include <cfg.h>

ModulePtr WindowModuleBuilder::build(Module& root, const cfg::json& config)
{
	// TODO
	return ModulePtr{};
}

REGISTER_MODULE_BUILDER(window, WindowModuleBuilder);
