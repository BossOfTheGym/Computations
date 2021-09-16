#include "shaders-module.h"

#include <cfg.h>

ModulePtr ShaderStorageModuleBuilder::build(Module& root, const cfg::json& config)
{
	// TODO
	return ModulePtr{};
}

REGISTER_MODULE_BUILDER(shader_storage, ShaderStorageModuleBuilder);