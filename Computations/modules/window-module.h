#pragma once

#include "module-builders.h"

class WindowModuleBuilder : public IModuleBuilder
{
public:
	ModulePtr build(Module& root, const cfg::json& config) override;
};
