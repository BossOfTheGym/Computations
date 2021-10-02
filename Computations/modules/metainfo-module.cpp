#include "metainfo-module.h"

#include <metainfo.h>

#include <stdexcept>

ModulePtr MetainfoModuleBuilder::build(Module& root, const cfg::json& config)
{
	if (!config.contains("metainfo")) {
		throw std::runtime_error("Missing config \"metainfo\".");
	}

	auto modulePtr = std::make_shared<Module>(
		Metainfo{
			.metainfo = config["metainfo"],
		}
	);

	if (auto [_, inserted] = root.load("metainfo", modulePtr); !inserted) {
		throw std::runtime_error("Module \"metainfo\" already loaded.");
	}
	return modulePtr;
}

REGISTER_MODULE_BUILDER(metainfo, MetainfoModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(metainfo);