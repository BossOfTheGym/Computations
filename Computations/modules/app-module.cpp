#include "app-module.h"

#include <cfg.h>
#include <app-params.h>

#include <stdexcept>

ModulePtr AppModuleBuilder::build(Module& root, const cfg::json& config)
{
	if (!config.contains("app")) {
		throw std::runtime_error("Missing config for module \"app\".");
	}
	auto& appConfig = config["app"];

	ModulePtr modulePtr = std::make_shared<Module>(
		AppParams{
			.xSplit = appConfig["x_split"].get<uint>(),
			.ySplit = appConfig["y_split"].get<uint>(),
			.totalUpdates = appConfig["total_updates"].get<uint>(),
			.itersPerUpdate = appConfig["iters_per_update"].get<uint>(),
			.gridX = appConfig["grid_x"].get<uint>(),
			.gridY = appConfig["grid_y"].get<uint>(),
		}
	);

	if (auto [_, inserted] = root.load("app", modulePtr); !inserted) {
		throw std::runtime_error("Module \"output\" already loaded");
	}
	return modulePtr;
}

REGISTER_MODULE_BUILDER(app, AppModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(app);
