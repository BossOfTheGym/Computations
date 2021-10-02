#pragma once

#include "module-builders.h"
#include "module-dependencies.h"

class MetainfoModuleBuilder : public IModuleBuilder
{
public:
	ModulePtr build(Module& root, const cfg::json& config) override;
};
