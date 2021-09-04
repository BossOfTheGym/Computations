#include "dirichlet_dataaabb2d.h"

namespace dir2d
{
	namespace
	{
		void initialize_solution_data(DataAabb2D& data, const DomainAabb2D& domain, const Function2D& boundary)
		{
			data.solution.reset(new f32[(domain.xSplit + 1) * (domain.ySplit + 1)]);

			auto ptr = data.solution.get();
			for (i32 j = 0; j <= domain.xSplit; j++) {
				f32 x = domain.x0 + j * domain.hx;

				*ptr++ = boundary(x, domain.y0);
			}
			for (i32 i = 1; i < domain.ySplit; i++) {
				f32 y = domain.y0 + i * domain.hy;

				*ptr++ = boundary(domain.x0, y);
				for (i32 j = 1; j < domain.xSplit; j++) {
					f32 x = domain.x0 + j * domain.hx;

					*ptr++ = 0.0;
				}
				*ptr++ = boundary(domain.x1, y);
			}
			for (i32 j = 0; j <= domain.xSplit; j++) {
				f32 x = domain.x0 + j * domain.hx;

				*ptr++ = boundary(x, domain.y1);
			}
		}

		void initialize_f_data(DataAabb2D& data, const DomainAabb2D& domain, const Function2D& f)
		{
			data.f.reset(new f32[(domain.xSplit + 1) * (domain.ySplit + 1)]);

			auto ptr = data.f.get();
			for (i32 j = 0; j <= domain.xSplit; j++) {
				*ptr++ = 0.0f;
			}
			for (i32 i = 1; i < domain.ySplit; i++) {
				f32 y = domain.y0 + i * domain.hy;

				*ptr++ = 0.0f;
				for (i32 j = 1; j < domain.xSplit; j++) {
					f32 x = domain.x0 + j * domain.hx;

					*ptr++ = f(x, y);
				}
				*ptr++ = 0.0;
			}
			for (i32 j = 0; j <= domain.xSplit; j++) {
				*ptr++ = 0.0f;
			}
		}
	}

	DataAabb2D DataAabb2D::create_data(const DomainAabb2D& domain, const Function2D& boundary, const Function2D& f)
	{
		DataAabb2D data;

		initialize_solution_data(data, domain, boundary);
		initialize_f_data(data, domain, f);

		return data;
	}
}
