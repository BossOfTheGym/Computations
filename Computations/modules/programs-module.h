#pragma once

#include <program-storage.h>

#include "module-builders.h"

class ProgramStorageModuleBuilder : public IModuleBuilder
{
public:
	ModulePtr build(Module& root, const cfg::json& config) override;
};
