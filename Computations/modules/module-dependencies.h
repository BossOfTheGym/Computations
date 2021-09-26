#pragma once

#include <string>
#include <vector>

#include <static-storage.h>

struct ModuleBuildDependencyTag;

using Dependencies = std::vector<std::string>;

template<class ... Deps>
Dependencies construct_dependencies(Deps&& ... deps)
{
	Dependencies deps;
	(deps.push_back(std::forward<Deps>(deps)), ...);
	return deps;
}

#define ACCESS_MODULE_BUILD_DEPENDENCIES(name, ...) ACCESS_STORAGE(ModuleBuildDependencyTag, Dependencies)
#define REGISTER_MODULE_BUILD_DEPENDENCY(name, ...) REGISTER_RESOURCE(ModuleBuildDependencyTag, Dependencies, name, construct_dependencies(#__VA_ARGS__))
