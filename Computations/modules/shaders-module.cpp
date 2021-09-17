#include "shaders-module.h"

#include <cfg.h>
#include <exception>

ModulePtr ShaderStorageModuleBuilder::build(Module& root, const cfg::json& config)
{
	if (!config.contains("shader_storage")) {
		throw std::runtime_error("No config found for module \"shader_storage\".");
	}

	if (!config["shader_storage"].contains("shader_folder")) {
		throw std::runtime_error("No option \"shader_folder\" found.");
	}
	auto shaderFolder = config["shader_storage"]["shader_folder"].get<std::string>();

	if (!config["shader_storage"].contains("shaders")) {
		throw std::runtime_error("No option \"shaders\" found.");
	}
	auto shaders = config["shader_storage"]["shaders"];

	// TODO : error log from shaders
	auto shaderStorage = ShaderStorage(shaderFolder);
	if (!shaderStorage.loadAll(shaders)) {
		// TODO : print error log
	}

	auto modulePtr = std::make_shared<Module>(std::move(shaderStorage));
	if (auto [_, inserted] = root.load("shader_storage", modulePtr); !inserted) {
		throw std::runtime_error("Module \"shader_storage\" already loaded.");
	}

	return modulePtr;
}

REGISTER_MODULE_BUILDER(shader_storage, ShaderStorageModuleBuilder);
