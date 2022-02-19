#pragma once

#include <cfg-fwd.h>
#include <static-storage.h>

#include "module-builder.h"

struct ModuleBuildersTag;

using IModuleBuilderPtr = std::unique_ptr<IModuleBuilder>;

#define ACCESS_MODULE_BUILDERS() ACCESS_STORAGE(ModuleBuildersTag, IModuleBuilderPtr)

#define REGISTER_MODULE_BUILDER(name, builder, ...) \
	REGISTER_RESOURCE(ModuleBuildersTag, IModuleBuilderPtr, name, std::make_unique<builder>(##__VA_ARGS__))
