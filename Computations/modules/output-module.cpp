#include "output-module.h"

#include <cfg.h>
#include <output-params.h>

#include <stdexcept>

ModulePtr OutputModuleBuilder::build(Module& root, const cfg::json& config)
{
	if (!config.contains("output")) {
		throw std::runtime_error("Missing config for module \"output\".");
	}
	auto& outputConfig = config["output"];

	if (!outputConfig.contains("output")) {
		throw std::runtime_error("Missing option \"output\" config.");
	}

	ModulePtr modulePtr = std::make_shared<Module>(
		OutputParams{
			.output = outputConfig["output"].get<std::string>(),
		}
	);

	if (auto [_, inserted] = root.load("output", modulePtr); !inserted) {
		throw std::runtime_error("Module \"output\" already loaded");
	}
	return modulePtr;
}

REGISTER_MODULE_BUILDER(output, OutputModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(output);
