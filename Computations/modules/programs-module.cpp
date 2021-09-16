#include "programs-module.h"

#include <cfg.h>

ModulePtr ProgramStorageModuleBuilder::build(Module& root, const cfg::json& config)
{
	// TODO
	return ModulePtr{};
}

REGISTER_MODULE_BUILDER(program_storage, ProgramStorageModuleBuilder);