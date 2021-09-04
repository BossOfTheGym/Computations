#include "dirichlet_domainaabb2d.h"

namespace dir2d
{
	bool DomainAabb2D::split_aligned(i32 split, i32 alignment)
	{
		return (split + 1) % alignment == 0;
	}

	i32 DomainAabb2D::align_split(i32 split, i32 alignment)
	{
		if (!split_aligned(split, alignment)) {
			split = (split + 1) - (split + 1) % alignment + alignment - 1;
		}
		return split;
	}

	DomainAabb2D DomainAabb2D::create_domain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
	{
		return DomainAabb2D{x0, x1, y0, y1, (x1 - x0) / xSplit, (y1 - y0) / ySplit, xSplit, ySplit};
	}

	DomainAabb2D DomainAabb2D::create_aligned_domain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit, i32 xAlign, i32 yAlign)
	{
		xSplit = align_split(xSplit, xAlign);
		ySplit = align_split(ySplit, yAlign);
		return DomainAabb2D{x0, x1, y0, y1, (x1 - x0) / xSplit, (y1 - y0) / ySplit, xSplit, ySplit};
	}

	bool DomainAabb2D::domain_aligned(const DomainAabb2D& domain, i32 xAlign, i32 yAlign)
	{
		return split_aligned(domain.xSplit, xAlign) && split_aligned(domain.ySplit, yAlign);
	}

	void DomainAabb2D::align_domain(DomainAabb2D& domain, i32 xAlign, i32 yAlign)
	{
		domain.xSplit = align_split(domain.xSplit, xAlign);
		domain.ySplit = align_split(domain.ySplit, yAlign);
	}
}