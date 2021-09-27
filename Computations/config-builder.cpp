#include "config-builder.h"

#include <string>
#include <stdexcept>

namespace
{
	void get_output_config(cfg::json& config, const std::string& output, uint xSplit, uint ySplit, uint workgroupSizeX, uint workgroupSizeY)
	{
		config["output"] = {
			{"output", output},
			{"x_split", xSplit},
			{"y_split", ySplit},
			{"workgroup_size_x", workgroupSizeX},
			{"workgroup_size_y", workgroupSizeY}
		};
	}

	void get_app_config(cfg::json& config, uint xSplit, uint ySplit)
	{
		config["app"] = {
			{"x_split", xSplit},
			{"y_split", ySplit},
			{"iters_per_update", 1},
			{"grid_x", 3},
			{"grid_y", 2},
		};
	}

	void get_dirichlet_config(cfg::json& config)
	{
		cfg::json dirichlet = {
			{"red_black", {}},
			{"red_black_tiled", {}},
			{"red_black_smt_s", {}},
			{"red_black_smtm_s", {}},
			{"red_black_smt", {}},
			{"red_black_smtm", {}}
		};

		config["dirichlet"] = dirichlet;
	}

	void get_shader_storage_config(cfg::json& config, uint workgroupSizeX, uint workgroupSizeY)
	{
		if (workgroupSizeX % 2 != 0 || workgroupSizeY % 2 != 0) {
			throw std::runtime_error("Workgroups dimensions must be even numbers.");
		}

		cfg::json wholeWorkgroup = {
			{"WORKGROUP_X", std::to_string(workgroupSizeX)},
			{"WORKGROUP_Y", std::to_string(workgroupSizeY)}
		};
		cfg::json halfWorkgroup = {
			{"WORKGROUP_X", std::to_string(workgroupSizeX / 2)},
			{"WORKGROUP_Y", std::to_string(workgroupSizeY / 2)}
		};

		cfg::json shaders;
		shaders["quad.frag"] = {};
		shaders["quad.vert"] = {};
		shaders["jacoby.comp"] = wholeWorkgroup;
		shaders["red_black.comp"] = wholeWorkgroup;
		shaders["red_black_smt_s.comp"] = halfWorkgroup;
		shaders["red_black_smtm_s.comp"] = halfWorkgroup;
		shaders["red_black_tiled.comp"] = halfWorkgroup;
		shaders["red_black_smtm_st0.comp"] = halfWorkgroup;
		shaders["red_black_smtm_st1.comp"] = halfWorkgroup;
		shaders["red_black_smt_st0.comp"] = halfWorkgroup;
		shaders["red_black_smt_st1.comp"] = halfWorkgroup;
		shaders["test_compute.comp"] = {};

		cfg::json shader_storage;
		shader_storage["shader_folder"] = "shaders";
		shader_storage["shaders"] = shaders;

		config["shader_storage"] = shader_storage;
	}

	void get_program_storage_config(cfg::json& config)
	{
		config["program_storage"] = {
			{"quad", cfg::json::array({"quad.frag", "quad.vert"})},
			{"jacoby", cfg::json::array({"jacoby.comp"})},
			{"red_black", cfg::json::array({"red_black.comp"})},
			{"red_black_tiled", cfg::json::array({"red_black_tiled.comp"})},
			{"red_black_smt_s", cfg::json::array({"red_black_smt_s.comp"})},
			{"red_black_smtm_s", cfg::json::array({"red_black_smtm_s.comp"})},
			{"red_black_smt_st0", cfg::json::array({"red_black_smt_st0.comp"})},
			{"red_black_smt_st1", cfg::json::array({"red_black_smt_st1.comp"})},
			{"red_black_smtm_st0", cfg::json::array({"red_black_smtm_st0.comp"})},
			{"red_black_smtm_st1", cfg::json::array({"red_black_smtm_st1.comp"})},
			{"test_compute", cfg::json::array({"test_compute.comp"})}
		};
	}

	void get_window_config(cfg::json& config)
	{
		config["window"] = {
			{"title", "computations"},
			{"height", 800},
			{"width", 1200},
		};
	}
}

cfg::json ConfigBuilder::build()
{
	cfg::json config;
	get_output_config(config, m_output, m_xSplit, m_ySplit, m_workgroupSizeX, m_workgroupSizeY);
	get_app_config(config, m_xSplit, m_ySplit);
	get_dirichlet_config(config);
	get_shader_storage_config(config, m_workgroupSizeX, m_workgroupSizeY);
	get_program_storage_config(config);
	get_window_config(config);
	return config;
}