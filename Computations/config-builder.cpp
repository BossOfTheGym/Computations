#include "config-builder.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace cfg;

namespace
{
	void get_output_config(json& config, const std::string& output)
	{
		config["output"] = {
			{"output", output},
		};
	}

	void get_app_config(json& config, uint xSplit, uint ySplit, uint totalUpdates, uint gridX, uint gridY)
	{
		config["app"] = {
			{"x_split", xSplit},
			{"y_split", ySplit},
			{"total_updates", totalUpdates},
			{"iters_per_update", 1},
			{"grid_x", gridX},
			{"grid_y", gridY},
		};
	}

	void get_meta_config(json& config, uint xSplit, uint ySplit, uint steps, uint workgroupSizeX, uint workgroupSizeY)
	{
		config["metainfo"] = {
			{"x_split", xSplit},
			{"y_split", ySplit},
			{"steps", steps},
			{"workgroup_size_x", workgroupSizeX},
			{"workgroup_size_y", workgroupSizeY},
		};
	}

	void get_dirichlet_config(json& config, const std::vector<std::string>& systems)
	{
		json dirichlet;	
		for (auto& sys : systems) {
			dirichlet[sys] = json::object();
		}
		config["dirichlet"] = dirichlet;
	}

	void get_shader_storage_config(json& config, uint workgroupSizeX, uint workgroupSizeY, uint steps)
	{
		if (workgroupSizeX % 2 != 0 || workgroupSizeY % 2 != 0) {
			throw std::runtime_error("Workgroups dimensions must be even numbers.");
		}

		// TODO : two config types are almost the same now
		json simpleConfig = {
			{"_CONFIGURED", ""},
			{"_WORKGROUP_X", std::to_string(workgroupSizeX)},
			{"_WORKGROUP_Y", std::to_string(workgroupSizeY)}
		};

		json tiledConfig = {
			{"_CONFIGURED", ""},
			{"_STEPS", std::to_string(steps)},
			{"_WORKGROUP_X", std::to_string(workgroupSizeX)},
			{"_WORKGROUP_Y", std::to_string(workgroupSizeY)}
		};
		
		json shaders;
		shaders["quad.frag"] = json::object();
		shaders["quad.vert"] = json::object();
		shaders["jacoby.comp"] = json::object({{"macros", simpleConfig}});
		shaders["red_black.comp"] = json::object({{"macros", simpleConfig}});
		shaders["red_black_tiled.comp"] = json::object({{"macros", tiledConfig}});
		shaders["red_black_smt_s.comp"] = json::object({{"macros", tiledConfig}});
		shaders["red_black_smtm_s.comp"] = json::object({{"macros", tiledConfig}});
		shaders["red_black_smtm_st0.comp"] = json::object({{"macros", tiledConfig}});
		shaders["red_black_smtm_st1.comp"] = json::object({{"macros", tiledConfig}});
		shaders["red_black_smtmo_st0.comp"] = json::object({{"macros", tiledConfig}});
		shaders["red_black_smtmo_st1.comp"] = json::object({{"macros", tiledConfig}});
		shaders["chaotic_tiled.comp"] = json::object({{"macros", tiledConfig}});
		shaders["chaotic_smtm_st0.comp"] = json::object({{"macros", tiledConfig}});
		shaders["chaotic_smtm_st1.comp"] = json::object({{"macros", tiledConfig}});
		shaders["test_compute.comp"] = json::object();

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
			{"red_black_smtm_s", json::array({"red_black_smtm_s.comp"})},
			{"red_black_smtm_st0", json::array({"red_black_smtm_st0.comp"})},
			{"red_black_smtm_st1", json::array({"red_black_smtm_st1.comp"})},
			{"red_black_smtmo_st0", json::array({"red_black_smtmo_st0.comp"})},
			{"red_black_smtmo_st1", json::array({"red_black_smtmo_st1.comp"})},
			{"chaotic_tiled", json::array({"chaotic_tiled.comp"})},
			{"chaotic_smtm_st0", json::array({"chaotic_smtm_st0.comp"})},
			{"chaotic_smtm_st1", json::array({"chaotic_smtm_st1.comp"})},
			{"test_compute", json::array({"test_compute.comp"})}
		};
	}

	void get_window_config(json& config, uint width, uint height)
	{
		config["window"] = {
			{"title", "computations"},
			{"height", height},
			{"width", width},
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
	get_output_config(config, m_output);
	get_app_config(config, m_xSplit, m_ySplit, m_totalUpdates, m_gridX, m_gridY);
	get_meta_config(config, m_xSplit, m_ySplit, m_steps, m_workgroupSizeX, m_workgroupSizeY);
	get_dirichlet_config(config, m_systems);
	get_shader_storage_config(config, m_workgroupSizeX, m_workgroupSizeY, m_steps);
	get_program_storage_config(config);
	get_window_config(config, m_windowWidth, m_windowHeight);
	get_glfw_config(config);
	return config;
}
