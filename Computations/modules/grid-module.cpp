#include "grid-module.h"
#include "module-dependencies.h"

#include <cfg.h>
#include <grid.h>
#include <app-params.h>
#include <main-window.h>
#include <window-builder.h>
#include <program-storage.h>

#include <stdexcept>

ModulePtr GridModuleBuilder::build(Module& root, const cfg::json& config)
{
	auto& app      = root.acquire("app")->get<AppParams>();
	auto& window   = root.acquire("window")->get<MainWindowPtr>();
	auto& programs = root.acquire("program_storage")->get<ProgramStorage>();

	auto [width, height] = window->size();

	auto program = programs.find("quad")->second.program.id;

	auto modulePtr = std::make_shared<Module>(placeholder_t<Grid>, app.gridX, app.gridY, width, height, program);
	if (auto [_, inserted] = root.load("grid", modulePtr); !inserted) {
		throw std::runtime_error("Module \"grid\" has been already loaded.");
	}
	return modulePtr;
}

REGISTER_MODULE_BUILDER(grid, GridModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(grid, "app", "window", "program_storage");