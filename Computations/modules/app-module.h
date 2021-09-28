#pragma once

#include "module-builders.h"
#include "module-dependencies.h"

class AppModuleBuilder : public IModuleBuilder
{
public:
	// json : {
	// 	   ...
	// 	   "app" : {
	//			"x_split" : uint,
	//			"y_split" : uint,
	//			"total_updates" : uint,
	//			"iters_per_update" : uint,
	//			"grid_x" : uint,
	//			"grid_y" : uint
	//		}
	//}
	ModulePtr build(Module& root, const cfg::json& config) override;
};
