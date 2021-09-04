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

	f32 compute_optimal_w(f32 hx, f32 hy, i32 xSplit, i32 ySplit);

	gl::Buffer create_work_buffer(uint workgroupsX, uint workgroupsY, uint size, bool pad = true);
}