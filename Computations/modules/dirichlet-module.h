#pragma once

#include "module-builders.h"

class DirichletModuleBuilder : public IModuleBuilder
{
public:
	ModulePtr build(Module& root, const cfg::json& config) override;
};
