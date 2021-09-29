#include "programs-module.h"
#include "module-dependencies.h"

#include <cfg.h>
#include <shader-storage.h>
#include <program-storage.h>
#include <shader-provider.h>

#include <exception>

namespace
{
	class ShaderProvider : public IShaderProvider
	{
	public:
		ShaderProvider(ShaderStorage& shaderStorage) : m_shaderStorage(shaderStorage)
		{}

		gl::Id get(const std::string& name) const override
		{
			auto it = m_shaderStorage.find(name);
			return (it != m_shaderStorage.end() ? it->second.id : gl::null);
		}

	private:
		ShaderStorage& m_shaderStorage;
	};
}

ModulePtr ProgramStorageModuleBuilder::build(Module& root, const cfg::json& config)
{
	auto it = root.find("shader_storage");
	if (it == root.end()) {
		throw std::runtime_error("Module \"shader_storage\" not found.");
	}

	auto& shaderStoragePtr = it->second;
	if (!shaderStoragePtr->stores<ShaderStorage>()) {
		throw std::runtime_error("Module \"shader_storage\" stores some inappropriate shit.");
	}
	auto shaderProvider = ShaderProvider(shaderStoragePtr->get<ShaderStorage>());
	
	if (!config.contains("program_storage")) {
		throw std::runtime_error("Missing configuration for module \"program_storage\".");
	}
	auto& programStorageConfig = config["program_storage"];

	auto programStorage = ProgramStorage();
	// TODO : program error log
	if (!programStorage.loadAll(programStorageConfig, shaderProvider)) {
		// TODO : print error log
	}

	auto modulePtr = std::make_shared<Module>(std::move(programStorage));
	if (auto [_, inserted] = root.load("program_storage", modulePtr); !inserted) {
		throw std::runtime_error("Module \"program_storage\" already loaded.");
	}

	return ModulePtr{};
}

REGISTER_MODULE_BUILDER(program_storage, ProgramStorageModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(program_storage, "shader_storage");
