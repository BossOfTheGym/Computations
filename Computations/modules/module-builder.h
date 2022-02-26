#pragma once

#include <cfg-fwd.h>

#include "module.h"

// TODO : little violation of single responsibility principple, this method also loads created module into the root module
// instead it should just return built module or empty ptr
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