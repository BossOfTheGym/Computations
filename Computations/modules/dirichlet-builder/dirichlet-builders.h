#pragma once

#include <static-storage.h>

#include <memory>

#include "../module-builder.h"

class ProgramStorage;

struct DirichletBuilderTag;

// TODO : little violation of single responsibility principple, this method also loads created module into the root module
// instead it should just return built module or empty ptr
class IDirichletBuilder
{
public:
	virtual ModulePtr build(Module& root, const cfg::json& config) = 0;
};

using IDirichletBuilderPtr = std::unique_ptr<IDirichletBuilder>;

#define ACCESS_DIRICHLET_BUILDERS() ACCESS_STORAGE(DirichletBuilderTag, IDirichletBuilderPtr)

#define REGISTER_DIRICHLET_BUILDER(name, builder, ...) \
	REGISTER_RESOURCE(DirichletBuilderTag, IDirichletBuilderPtr, name, std::make_unique<builder>(##__VA_ARGS__))