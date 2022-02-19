#pragma once

#include <cfg.h>
#include <parse-util.h>
#include <program-storage.h>
#include <dirichlet-params.h>

#include <dirichlet/dirichlet-proxy.h>

#include "../module.h"

#include <string>
#include <exception>
#include <tuple>

#define LAZY_CPP_EVASION inline

using namespace cfg;
using namespace std::string_literals;

LAZY_CPP_EVASION
gl::Id get_shader_program(ProgramStorage& storage, const std::string& name)
{
	auto shaderProgramIt = storage.find(name);
	if (shaderProgramIt == storage.end())
	{
		throw std::runtime_error("Failed to obtain \"" + name + "\" program.");
	}
	return shaderProgramIt->second.program.id;
}

template<class T>
LAZY_CPP_EVASION
T parse_value(const json& js, const std::string& key)
{
	T value;
	if (!parse(js[key].get<std::string>(), value))
	{
		throw std::runtime_error("Failed to parse json value \"" + key + "\"");
	}
	return value;
}

LAZY_CPP_EVASION
const json& try_get_value(const json& js, const std::string& key)
{
	if (!js.contains(key))
	{
		throw std::runtime_error("Missing config \"" + key + "\".");
	}
	return js[key];
}

LAZY_CPP_EVASION
const json& try_get_value(const json& js, const json::json_pointer& key)
{
	if (!js.contains(key))
	{
		throw std::runtime_error("Missing config \"" + key.to_string() + "\".");
	}
	return js[key];
}

LAZY_CPP_EVASION
ModulePtr try_get_module(Module& root, const std::string& name)
{
	if (auto it = root.find(name); it != root.end())
	{
		return it->second;
	}
	throw std::runtime_error("No module \"" + name + "\" found.");
}

template<class T>
LAZY_CPP_EVASION
T& try_get_module_data(Module& root, const std::string& name)
{
	return try_get_module(root, name)->get<T>();
}

LAZY_CPP_EVASION
void try_load_module(Module& root, ModulePtr toLoad, const std::string& name)
{
	if (auto [_, inserted] = root.load(name, toLoad); !inserted)
	{
		throw std::runtime_error("Module \"" + name + "\" already loaded.");
	}
}

LAZY_CPP_EVASION
std::tuple<ModulePtr, ModulePtr> try_get_dirichlet_parts(Module& root)
{
	auto dirichlet = try_get_module(root, "dirichlet");
	return std::make_tuple(try_get_module(*dirichlet, "systems"),
						try_get_module(*dirichlet, "controls"));
}

template<class System>
ModulePtr create_one_shader_sys(Module& systems,
								Module& controls,
								ProgramStorage& storage,
								const json& config,
								const std::string& name,
								const std::string& prog)
{
	auto& systemConfig = try_get_value(config, json::json_pointer("/dirichlet/" + name));
	auto& shaderConfig = try_get_value(config, json::json_pointer("/shader_storage/shaders/" + prog + ".comp"));

	uint workgroupX = parse_value<uint>(shaderConfig["macros"], "_WORKGROUP_X");
	uint workgroupY = parse_value<uint>(shaderConfig["macros"], "_WORKGROUP_Y");
	gl::Id programId = get_shader_program(storage, prog);

	ModulePtr systemModule = std::make_shared<Module>(placeholder_t<System>, workgroupX, workgroupY, programId);
	try_load_module(systems, systemModule, name);

	ModulePtr systemProxy = std::make_shared<Module>(placeholder_t<dir2d::Proxy>, systemModule->get<System>());
	try_load_module(controls, systemProxy, name);

	return systemModule;
}

template<class System>
ModulePtr create_two_shader_sys(Module& systems,
								Module& controls,
								ProgramStorage& storage,
								const json& config,
								const std::string& name,
								const std::string& prog0,
								const std::string& prog1)
{
	auto& systemConfig = try_get_value(config, json::json_pointer("/dirichlet/" + name));
	auto& shaderConfig0 = try_get_value(config, json::json_pointer("/shader_storage/shaders/" + prog0 + ".comp"));
	auto& shaderConfig1 = try_get_value(config, json::json_pointer("/shader_storage/shaders/" + prog1 + ".comp"));

	uint workgroupX0 = parse_value<uint>(shaderConfig0["macros"], "_WORKGROUP_X");
	uint workgroupY0 = parse_value<uint>(shaderConfig0["macros"], "_WORKGROUP_Y");
	gl::Id programId0 = get_shader_program(storage, prog0);

	uint workgroupX1 = parse_value<uint>(shaderConfig1["macros"], "_WORKGROUP_X");
	uint workgroupY1 = parse_value<uint>(shaderConfig1["macros"], "_WORKGROUP_Y");
	gl::Id programId1 = get_shader_program(storage, prog1);

	if (workgroupX0 != workgroupX1 || workgroupY0 != workgroupY1)
	{
		throw std::runtime_error("Invalid workgroup dimensions specified: they must be equal in both programs.");
	}

	ModulePtr systemModule = std::make_shared<Module>(placeholder_t<System>, workgroupX0, workgroupY0, programId0, programId1);
	try_load_module(systems, systemModule, name);

	ModulePtr systemProxy = std::make_shared<Module>(placeholder_t<dir2d::Proxy>, systemModule->get<System>());
	try_load_module(controls, systemProxy, name);

	return systemModule;
}