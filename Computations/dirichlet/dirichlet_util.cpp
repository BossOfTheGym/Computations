#include "dirichlet_util.h"

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

namespace dir2d
{
	NumWorkgroups get_num_workgroups(uint splitX, uint splitY, uint workgroupSizeX, uint workgroupSizeY)
	{
		return {splitX / workgroupSizeX + 1, splitY / workgroupSizeY + 1};
	}

	f32 compute_optimal_w(f32 hx, f32 hy, i32 xSplit, i32 ySplit)
	{
		i32 sx = xSplit;
		i32 sy = ySplit;

		f32 hxhx = hx * hx;
		f32 hyhy = hy * hy;
		f32 H = hxhx + hyhy;
		f32 sinx = std::sin(pid2 / sx);
		f32 siny = std::sin(pid2 / sy);

		f32 delta = 2.0 * hxhx / H * sinx * sinx + 2.0 * hyhy / H * siny * siny;

		return 2.0 / (1.0 + std::sqrt(delta * (2.0 - delta)));
	}

	gl::Buffer create_work_buffer(uint workgroupsX, uint workgroupsY, uint size, bool pad)
	{
		if (pad) {
			workgroupsX += 2;
			workgroupsY += 2;
		}

		uint storageSize = workgroupsX * workgroupsY * size;

		return gl::create_storage_buffer(storageSize, 0, nullptr);
	}
}