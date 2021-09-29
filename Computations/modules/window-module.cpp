#include "window-module.h"
#include "module-dependencies.h"

#include <cfg.h>
#include <window-builder.h>

#include <exception>

ModulePtr WindowModuleBuilder::build(Module& root, const cfg::json& config)
{
	if (!config.contains("window")) {
		throw std::runtime_error("No config found for module \"window\"");
	}

	auto windowPtr = WindowBuilder().build(config["window"]);
	if (!windowPtr->valid()) {
		throw std::runtime_error("Failed to create window.");
	}

	auto modulePtr = std::make_shared<Module>(std::move(windowPtr));
	if (auto [_, inserted] = root.load("window", modulePtr); !inserted) {
		throw std::runtime_error("Module \"window\" already loaded");
	}
	return modulePtr;
}

REGISTER_MODULE_BUILDER(window, WindowModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(window, "glfw");