#pragma once

#include <core.h>
#include <gl-cxx/gl-res.h>
#include <gl-cxx/gl-types.h>

namespace dir2d
{
	struct NumWorkgroups
	{
		uint numX{};
		uint numY{};
	};

	NumWorkgroups get_num_workgroups(uint splitX, uint splitY, uint workgroupSizeX, uint workgroupSizeY);


	f32 compute_optimal_w(f32 hx, f32 hy, int xSplit, int ySplit);

	gl::Buffer create_work_buffer(uint workgroupsX, uint workgroupsY, uint size, bool pad = true);


	enum class Stage
	{
		Stage0,
		Stage1,
	};
	
	// example grig 5x5:
	// 0 1 | 0 1 | 0
	// -------------
	// 1 0 | 1 0 | 1
	// 0 1 | 0 1 | 0
	// -------------
	// 1 0 | 1 0 | 1
	// 0 1 | 0 1 | 0
	uint count_stage_workgroups(uint workgroupsX, uint workgroupsY, Stage stage);
}