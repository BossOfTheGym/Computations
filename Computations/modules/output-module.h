#pragma once

#include "module-builders.h"
#include "module-dependencies.h"

class OutputModuleBuilder : public IModuleBuilder
{
public:
	// json : {
	// 	   ...
	// 	   "output" : {
	//			"output" : <output_file>
	//		}
	//}
	ModulePtr build(Module& root, const cfg::json& config) override;
};
