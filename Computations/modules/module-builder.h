#pragma once

#include <cfg-fwd.h>

#include "module.h"

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