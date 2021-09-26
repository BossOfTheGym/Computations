#include "glfw-module.h"
#include "module-dependencies.h"

#include <cfg.h>
#include <glfw-guard.h>

#include <exception>

ModulePtr GlfwModuleBuilder::build(Module& root, const cfg::json& config)
{
	if (!config.contains("glfw")) {
		throw std::runtime_error("Missing config \"glfw\".");
	}

	auto modulePtr = std::make_shared<Module>(placeholder_t<GlfwGuard>);
	if (auto [_, inserted] = root.load("glfw", modulePtr); !inserted) {
		throw std::runtime_error("Module \"glfw\" has been already loaded.");
	}
	return modulePtr;
}

REGISTER_MODULE_BUILDER(glfw, GlfwModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(glfw);