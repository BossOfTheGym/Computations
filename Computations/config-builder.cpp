#include "config-builder.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace cfg;

namespace
{
	void get_output_config(json& config, const std::string& output, uint xSplit, uint ySplit, uint workgroupSizeX, uint workgroupSizeY)
	{
		config["output"] = {
			{"output", output},
			{"x_split", xSplit},
			{"y_split", ySplit},
			{"workgroup_size_x", workgroupSizeX},
			{"workgroup_size_y", workgroupSizeY}
		};
	}

	void get_app_config(json& config, uint xSplit, uint ySplit)
	{
		config["app"] = {
			{"x_split", xSplit},
			{"y_split", ySplit},
			{"iters_per_update", 1},
			{"grid_x", 3},
			{"grid_y", 2},
		};
	}

	void get_dirichlet_config(json& config)
	{
		json dirichlet = {
			{"red_black", json::object()},
			{"red_black_tiled", json::object()},
			{"red_black_smt_s", json::object()},
			{"red_black_smtm_s", json::object()},
			{"red_black_smt", json::object()},
			{"red_black_smtm", json::object()}
		};

		config["dirichlet"] = dirichlet;
	}

	void get_shader_storage_config(json& config, uint workgroupSizeX, uint workgroupSizeY)
	{
		if (workgroupSizeX % 2 != 0 || workgroupSizeY % 2 != 0) {
			throw std::runtime_error("Workgroups dimensions must be even numbers.");
		}

		json wholeWorkgroup = {
			{"WORKGROUP_X", std::to_string(workgroupSizeX)},
			{"WORKGROUP_Y", std::to_string(workgroupSizeY)}
		};
		json halfWorkgroup = {
			{"WORKGROUP_X", std::to_string(workgroupSizeX / 2)},
			{"WORKGROUP_Y", std::to_string(workgroupSizeY / 2)}
		};
		
		json shaders;
		shaders["quad.frag"]               = json::object();
		shaders["quad.vert"]               = json::object();
		shaders["jacoby.comp"]             = json::object({{"macros", wholeWorkgroup}});
		shaders["red_black.comp"]          = json::object({{"macros", wholeWorkgroup}});
		shaders["red_black_smt_s.comp"]    = json::object({{"macros", halfWorkgroup}});
		shaders["red_black_smtm_s.comp"]   = json::object({{"macros", halfWorkgroup}});
		shaders["red_black_tiled.comp"]    = json::object({{"macros", halfWorkgroup}});
		shaders["red_black_smtm_st0.comp"] = json::object({{"macros", halfWorkgroup}});
		shaders["red_black_smtm_st1.comp"] = json::object({{"macros", halfWorkgroup}});
		shaders["red_black_smt_st0.comp"]  = json::object({{"macros", halfWorkgroup}});
		shaders["red_black_smt_st1.comp"]  = json::object({{"macros", halfWorkgroup}});
		shaders["test_compute.comp"]       = json::object();

		json shader_storage;
		shader_storage["shader_folder"] = "shaders";
		shader_storage["shaders"] = shaders;

		config["shader_storage"] = shader_storage;
	}

	void get_program_storage_config(json& config)
	{
		config["program_storage"] = {
			{"quad", json::array({"quad.frag", "quad.vert"})},
			{"jacoby", json::array({"jacoby.comp"})},
			{"red_black", json::array({"red_black.comp"})},
			{"red_black_tiled", json::array({"red_black_tiled.comp"})},
			{"red_black_smt_s", json::array({"red_black_smt_s.comp"})},
			{"red_black_smtm_s", json::array({"red_black_smtm_s.comp"})},
			{"red_black_smt_st0", json::array({"red_black_smt_st0.comp"})},
			{"red_black_smt_st1", json::array({"red_black_smt_st1.comp"})},
			{"red_black_smtm_st0", json::array({"red_black_smtm_st0.comp"})},
			{"red_black_smtm_st1", json::array({"red_black_smtm_st1.comp"})},
			{"test_compute", json::array({"test_compute.comp"})}
		};
	}

	void get_window_config(json& config)
	{
		config["window"] = {
			{"title", "computations"},
			{"height", 800},
			{"width", 1200},
		};
	}

	void get_glfw_config(json& config)
	{
		config["glfw"] = json::object();
	}
}

json ConfigBuilder::build()
{
	json config;
	get_output_config(config, m_output, m_xSplit, m_ySplit, m_workgroupSizeX, m_workgroupSizeY);
	get_app_config(config, m_xSplit, m_ySplit);
	get_dirichlet_config(config);
	get_shader_storage_config(config, m_workgroupSizeX, m_workgroupSizeY);
	get_program_storage_config(config);
	get_window_config(config);
	get_glfw_config(config);

	// DEBUG
	std::cout << std::setw(4) << config << std::endl; 

	return config;
}