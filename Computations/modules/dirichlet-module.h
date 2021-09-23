#pragma once

#include "module-builders.h"

class DirichletModuleBuilder : public IModuleBuilder
{
public:
	// json : {
	// 	   ...
	//		"dirichlet" : {
	//			"x_split" : <uint>,
	//			"y_split" : <uint>,
	//			"iters_per_update" : <uint>,
	// 
	//			"systems" : {
	//				...
	//				<system_name> : {
	//					<whatever configs system has>
	// 				},
	// 				...
	//			}
	//		},
	// 	   ...
	// }
	// 
	// system may look up config options from configs of other modules (particularly shader_storage)
	//
	// module structure (submodules):
	// "systems" : <all systems are stored in this submodule>
	// "control" : <proxies that control systems (create & destroy domain, update them and get elapsed time)>
	// 
	// TODO : this module should be split up to several modules
	ModulePtr build(Module& root, const cfg::json& config) override;
};
