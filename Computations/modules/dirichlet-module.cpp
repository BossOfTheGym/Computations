#include "dirichlet-module.h"
#include "module-dependencies.h"

#include <cfg.h>
#include <parse-util.h>
#include <program-storage.h>
#include <dirichlet-params.h>

#include <dirichlet/dirichlet-proxy.h>
#include <dirichlet/jacoby.h>
#include <dirichlet/red_black.h>
#include <dirichlet/red_black_tiled.h>
#include <dirichlet/red_black_smt.h>
#include <dirichlet/red_black_smtm.h>
#include <dirichlet/red_black_smt_s.h>
#include <dirichlet/red_black_smtm_s.h>

#include <string>
#include <exception>

// TODO : I'm such a lazy ass so I just don't want to create a shittonne of additional builder-like classes
// so there wil be just a bunch of functions
// TODO : most probable builder should accept something like IProgramProvider as "program_storage" module does.
// But I'm fucking lazy and just 'll pass storage directly into create_* function

using namespace cfg;
using namespace std::string_literals;

namespace
{
	// TODO : all this utilities must be part of something
	gl::Id get_shader_program(ProgramStorage& storage, const std::string& name)
	{
		auto shaderProgramIt = storage.find(name);
		if (shaderProgramIt == storage.end()) {
			throw std::runtime_error("Failed to obtain \"" + name + "\" program.");
		}
		return shaderProgramIt->second.program.id;
	}

	template<class T>
	T parse_value(const json& js, const std::string& key)
	{
		T value;
		if (!parse(js[key].get<std::string>(), value)) {
			throw std::runtime_error("Failed to parse json value \"" + key + "\"");
		}
		return value;
	}

	const json& try_get_value(const json& js, const std::string& key) 
	{
		if (!js.contains(key)) {
			throw std::runtime_error("Missing config \"" + key+ "\".");
		}
		return js[key];
	}

	const json& try_get_value(const json& js, const json::json_pointer& key)
	{
		if (!js.contains(key)) {
			throw std::runtime_error("Missing config \"" + key.to_string() + "\".");
		}
		return js[key];
	}

	ModulePtr try_get_module(Module& root, const std::string& name)
	{
		if (auto it = root.find(name); it != root.end()) {
			return it->second;
		}
		throw std::runtime_error("No module \"" + name + "\" found.");
	}

	template<class T>
	T& try_get_module_data(Module& root, const std::string& name)
	{
		return try_get_module(root, name)->get<T>();
	}

	void try_load_module(Module& root, ModulePtr toLoad, const std::string& name)
	{
		if (auto [_, inserted] = root.load(name, toLoad); !inserted) {
			throw std::runtime_error("Module \"" + name + "\" already loaded.");
		}
	}


	template<class System>
	void create_one_shader_sys(
		Module& root,
		Module& systems,
		Module& controls,
		ProgramStorage& storage,
		const cfg::json& config,
		const std::string& name,
		const std::string& prog)
	{
		auto& systemConfig = try_get_value(config, json::json_pointer("dirichlet/systems/" + name));
		auto& shaderConfig = try_get_value(config, json::json_pointer("shader_storage/shaders/" + prog + ".comp"));

		uint workgroupX = parse_value<uint>(shaderConfig, "WORKGROUP_X");
		uint workgroupY = parse_value<uint>(shaderConfig, "WORKGROUP_Y");
		gl::Id programId = get_shader_program(storage, prog);

		ModulePtr systemModule = std::make_shared<Module>(placeholder_t<System>, workgroupX, workgroupY, programId);
		try_load_module(systems, systemModule, name);

		ModulePtr systemProxy = std::make_shared<Module>(placeholder_t<dir2d::Proxy>, systemModule->get<System>());
		try_load_module(controls, systemProxy, name);
	}

	template<class System>
	void create_two_shader_sys(
		Module& root,
		Module& systems,
		Module& controls,
		ProgramStorage& storage,
		const std::string& name,
		const std::string& prog0,
		const std::string& prog1,
		const cfg::json& config)
	{
		auto& systemConfig = try_get_value(config, json::json_pointer("dirichlet/systems/" + name));
		auto& shaderConfig0 = try_get_value(config, json::json_pointer("shader_storage/shaders/" + prog0 + ".comp"));
		auto& shaderConfig1 = try_get_value(config, json::json_pointer("shader_storage/shaders/" + prog1 + ".comp"));

		uint workgroupX0 = parse_value<uint>(shaderConfig0, "WORKGROUP_X");
		uint workgroupY0 = parse_value<uint>(shaderConfig0, "WORKGROUP_Y");
		gl::Id programId0 = get_shader_program(storage, prog0);

		uint workgroupX1 = parse_value<uint>(shaderConfig1, "WORKGROUP_X");
		uint workgroupY1 = parse_value<uint>(shaderConfig1, "WORKGROUP_Y");
		gl::Id programId1 = get_shader_program(storage, prog1);

		if (workgroupX0 != workgroupX1 || workgroupY0 != workgroupY1) {
			throw std::runtime_error("Invalid workgroup dimensions specified: they must be equal in both programs.");
		}

		ModulePtr systemModule = std::make_shared<Module>(placeholder_t<System>, workgroupX0, workgroupY0, programId0, programId1);
		try_load_module(systems, systemModule, name);

		ModulePtr systemProxy = std::make_shared<Module>(placeholder_t<dir2d::Proxy>, systemModule->get<System>());
		try_load_module(controls, systemProxy, name);
	}
}

ModulePtr DirichletModuleBuilder::build(Module& root, const cfg::json& config)
{
	auto& dirichlet = try_get_value(config, "dirichlet");
	auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");

	auto modulePtr  = std::make_shared<Module>();
	auto systemsPtr = std::make_shared<Module>();
	auto controlsPtr = std::make_shared<Module>();

	try_load_module(*modulePtr, systemsPtr, "systems");
	try_load_module(*modulePtr, controlsPtr, "controls");
	try_load_module(root, modulePtr, "dirichlet");

	// TODO : can be definitely registered & loaded automatically
	//create_one_shader_sys<dir2d::Jacoby>(root, *systemsPtr, *controlsPtr, programStorage, config, "jacoby", "jacoby");
	create_one_shader_sys<dir2d::RedBlack>(root, *systemsPtr, *controlsPtr, programStorage, config, "red_black", "red_black");
	create_one_shader_sys<dir2d::RedBlackTiled>(root, *systemsPtr, *controlsPtr, programStorage, config, "red_black_tiled", "red_black_tiled");
	create_one_shader_sys<dir2d::RedBlackTiledSmtS>(root, *systemsPtr, *controlsPtr, programStorage, config, "red_black_smt_s", "red_black_smt_s");
	create_one_shader_sys<dir2d::RedBlackTiledSmtmS>(root, *systemsPtr, *controlsPtr, programStorage, config, "red_black_smtm_s", "red_black_smtm_s");

	create_two_shader_sys<dir2d::RedBlackTiledSmt>(root, *systemsPtr, *controlsPtr, programStorage, config, "red_black_smt", "red_black_smt_st0", "red_black_smt_st1");
	create_two_shader_sys<dir2d::RedBlackTiledSmtm>(root, *systemsPtr, *controlsPtr, programStorage, config, "red_black_smtm", "red_black_smtm_st0", "red_black_smtm_st1");

	return modulePtr;
}

REGISTER_MODULE_BUILDER(dirichlet, DirichletModuleBuilder);
REGISTER_MODULE_BUILD_DEPENDENCY(dirichlet, program_storage);
