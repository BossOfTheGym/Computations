#pragma once

#include "module-builders.h"

class GridModuleBuilder : public IModuleBuilder
{
public:
	// no config required
	ModulePtr build(Module& root, const cfg::json& config) override;
};
