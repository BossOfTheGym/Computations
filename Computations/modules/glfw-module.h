#pragma once

#include "module-builders.h"

class GlfwModuleBuilder : public IModuleBuilder
{
public:
	// json : {
	// 	   ...
	// 	   "glfw" : {}
	// 	   ...
	// }
	ModulePtr build(Module& root, const cfg::json& config) override;
};
