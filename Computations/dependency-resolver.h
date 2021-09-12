#pragma once

#include "dependency.h"

class DependencyResolver
{
public:
	std::vector<Dependency*> resolve();
};