#pragma once

#include <cfg-fwd.h>
#include <static-storage.h>

#include "module.h"

struct ModuleBuildersTag;

class IModuleBuilder
{
public:
	virtual ~IModuleBuilder() = default;

	// json : {
	//		...
	//		<module_name_i> : {
	//			<module_config_i>
	//		}
	//		...
	// }
	//
	// root : Module root containing all modules
	virtual ModulePtr build(Module& root, const cfg::json& config) = 0;
};

using IModuleBuilderPtr = std::unique_ptr<IModuleBuilder>;

#define ACCESS_MODULE_BUILDERS() ACCESS_STORAGE(ModuleBuildersTag, IModuleBuilderPtr)

#define REGISTER_MODULE_BUILDER(name, builder, ...) REGISTER_RESOURCE(ModuleBuildersTag, IModuleBuilderPtr, name, std::make_unique<builder>(##__VA_ARGS__))
