#pragma once

#include "module-builders.h"

class ShaderStorageModuleBuilder : public IModuleBuilder
{
public:
	// "shader_storage" : {
	//		"shader_folder" : <shader_folder>,
	// 	    "shaders" : {
	//			...
	//			<shaders_config_i>
	//			...
	//		}
	//  }
	//
	// root : root module to store loaded module in
	ModulePtr build(Module& root, const cfg::json& config) override;
};
