#pragma once

#include <shader-storage.h>

#include "module-builders.h"

class ShaderStorageModuleBuilder : public IModuleBuilder
{
public:
	ModulePtr build(Module& root, const cfg::json& config) override;
};
