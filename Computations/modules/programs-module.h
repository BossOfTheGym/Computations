#pragma once

#include "module-builders.h"

class ProgramStorageModuleBuilder : public IModuleBuilder
{
public:
	// json : {
	// 	   ...
	// 	   "program_storage" : {
	//			...
	//			<program_config_i>
	// 			...
	// 	   }
	// }
	ModulePtr build(Module& root, const cfg::json& config) override;
};
