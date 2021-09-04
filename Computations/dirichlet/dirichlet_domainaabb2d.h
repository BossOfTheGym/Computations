#pragma once

#include <core.h>

namespace dir2d
{
	struct DomainAabb2D
	{
		static bool split_aligned(i32 split, i32 alignment);

		static i32 align_split(i32 split, i32 alignment);

		static DomainAabb2D create_domain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit);

		static DomainAabb2D create_aligned_domain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit, i32 xAlign, i32 yAlign);

		static bool domain_aligned(const DomainAabb2D& domain, i32 xAlign, i32 yAlign);

		static void align_domain(DomainAabb2D& domain, i32 xAlign, i32 yAlign);

		f32 x0{};
		f32 x1{};
		f32 y0{};
		f32 y1{};
		f32 hx{};
		f32 hy{};
		i32 xSplit{};
		i32 ySplit{};
	};
}