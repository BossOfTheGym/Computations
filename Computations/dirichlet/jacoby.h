#pragma once

#include <core.h>
#include <handle.h>
#include <storage.h>
#include <handle-pool.h>

#include <gl-cxx/gl-res.h>
#include <gl-cxx/gl-types.h>

#include "time_query.h"
#include "dirichlet_handle.h"
#include "resource_provider.h"
#include "dirichlet_dataaabb2d.h"
#include "dirichlet_domainaabb2d.h"

namespace dir2d
{
	class Jacoby 
		: public HandlePool
		, public SmartHandleProvider
		, public IResourceProvider
	{
	public:
		struct Uniforms
		{
			Uniforms(gl::Id program);

			void setup(gl::Id program);
			bool valid() const;

			GLint curr{-1};
			GLint hx{-1};
			GLint hy{-1};
		};

		struct Solution
		{
			static bool create(Solution& solution, const DomainAabb2D& domain, const DataAabb2D& data);

			gl::Id texture() const;
			void pingpong(); // curr ^= 1

			gl::Texture s[2]; // s = solution
			gl::Texture f; // f - see problem description
			int curr{};
		};

		struct UpdateParams
		{
			uint itersPerUpdate{};
		};

	public:
		Jacoby(uint workgroupSizeX, uint workgroupSizeY, gl::Id program);

		~Jacoby() = default;

		Jacoby(const Jacoby&) = delete;
		Jacoby& operator = (const Jacoby&) = delete;

		Jacoby(Jacoby&&) noexcept = delete;
		Jacoby& operator = (Jacoby&&) noexcept = delete;

	public:
		Handle create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config);
		SmartHandle createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config);

	public: // IResourceProvider
		bool valid(Handle handle) const override;
		void destroy(Handle handle);

		const DomainAabb2D& domain(Handle handle) const override;
		gl::Id texture(Handle handle) const override;

	public:
		void update();

		GLuint64 elapsed() const;
		f64 elapsedMean() const;

	private:
		uint m_workgroupSizeX{};
		uint m_workgroupSizeY{};

		gl::Id m_program;
		Uniforms m_uniforms;
		TimeQuery m_query;

		Storage<DomainAabb2D> m_domainStorage;
		Storage<Solution>     m_solutionStorage;
		Storage<UpdateParams> m_configStorage;
	};
}
