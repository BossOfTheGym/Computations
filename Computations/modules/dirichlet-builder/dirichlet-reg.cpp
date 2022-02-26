#include "dirichlet-builder-util.h"

#include "dirichlet-builders.h"

#include <dirichlet/jacoby.h>
#include <dirichlet/red_black.h>
#include <dirichlet/chaotic_smtm.h>
#include <dirichlet/chaotic_tiled.h>
#include <dirichlet/red_black_smtm.h>
#include <dirichlet/red_black_smtmo.h>
#include <dirichlet/red_black_tiled.h>
#include <dirichlet/red_black_smtm_s.h>

#include <program-storage.h>

class JacobyBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/jacoby"_json_pointer)) {
			return create_one_shader_sys<dir2d::Jacoby>(*systems,
													 *controls,
													 programStorage,
													 config,
													 "jacoby",
													 "jacoby");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(jacoby, JacobyBuilder);

class RedBlackBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/red_black"_json_pointer)) {
			return create_one_shader_sys<dir2d::RedBlack>(*systems,
														*controls,
														programStorage,
														config,
														"red_black",
														"red_black");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(red_black, RedBlackBuilder);

class RedBlackTiledBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/red_black_tiled"_json_pointer)) {
			return create_one_shader_sys<dir2d::RedBlackTiled>(*systems,
															   *controls,
															   programStorage,
															   config,
															   "red_black_tiled",
															   "red_black_tiled");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(red_black_tiled, RedBlackTiledBuilder);

class RedBlackSmtmBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/red_black_smtm"_json_pointer))
		{
			return create_two_shader_sys<dir2d::RedBlackTiledSmtm>(*systems,
																   *controls,
																   programStorage,
																   config,
																   "red_black_smtm",
																   "red_black_smtm_st0",
																   "red_black_smtm_st1");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(red_black_smtm, RedBlackSmtmBuilder);

class RedBlackSmtmSBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/red_black_smtm_s"_json_pointer))
		{
			return create_one_shader_sys<dir2d::RedBlackTiledSmtmS>(*systems,
																	*controls,
																	programStorage,
																	config,
																	"red_black_smtm_s",
																	"red_black_smtm_s");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(red_black_smtm_s, RedBlackSmtmSBuilder);

class RedBlackSmtmoBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/red_black_smtmo"_json_pointer)) {
			return create_two_shader_sys<dir2d::RedBlackTiledSmtmo>(*systems,
																	*controls,
																	programStorage,
																	config,
																	"red_black_smtmo",
																	"red_black_smtmo_st0",
																	"red_black_smtmo_st1");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(red_black_smtmo, RedBlackSmtmoBuilder);

class ChaoticTiledBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/chaotic_tiled"_json_pointer)) {
			return create_one_shader_sys<dir2d::ChaoticTiled>(*systems,
															  *controls,
															  programStorage,
															  config,
															  "chaotic_tiled",
															  "chaotic_tiled");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(chaotic_tiled, ChaoticTiledBuilder);

class ChaoticSmtmBuilder : public IDirichletBuilder
{
	ModulePtr build(Module& root, const json& config) override
	{
		auto& programStorage = try_get_module_data<ProgramStorage>(root, "program_storage");
		auto [systems, controls] = try_get_dirichlet_parts(root);

		if (config.contains("/dirichlet/chaotic_smtm"_json_pointer)) {
			return create_two_shader_sys<dir2d::ChaoticSmtm>(*systems,
															 *controls,
															 programStorage,
															 config,
															 "chaotic_smtm",
															 "chaotic_smtm_st0",
															 "chaotic_smtm_st1");
		}
		return {};
	}
};

REGISTER_DIRICHLET_BUILDER(chaotic_smtm, ChaoticSmtmBuilder);