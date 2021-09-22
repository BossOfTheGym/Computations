#pragma once

#include "red_black.h"

namespace dir2d
{
	class RedBlackTiledSmtm
		: HandlePool
		, SmartHandleProvider
		, IResourceProvider
	{
	public:
		struct Uniforms
		{
			Uniforms(gl::Id program);

			void setup(gl::Id program);
			bool valid() const;

			GLint curr{-1};
			GLint w{-1};
			GLint hx{-1};
			GLint hy{-1};
			GLint numWorkgroupsX{-1};
			GLint numWorkgroupsY{-1};
		};

		struct Solution
		{
			static bool create(
				Solution& solution,
				const DomainAabb2D& domain,
				const DataAabb2D& data,
				uint workgroupSizeX,
				uint workgroupSizeY);

			gl::Id texture() const;
			void pingpong();

			gl::Texture s[2];         // solution
			gl::Texture intermediate; // intermediate solution
			gl::Texture f;            // f-function from description

			i32 curr{};
			f32 w{};
		};

		//struct UpdateParams
		//{
		//	uint itersPerUpdate{}; // if zero then no update. updated once otherwise
		//};

	public:
		RedBlackTiledSmtm(uint workgroupSizeX, uint workgroupSizeY, gl::Id programSt0, gl::Id programSt1);

		~RedBlackTiledSmtm() = default;

		RedBlackTiledSmtm(const RedBlackTiledSmtm&) = delete;
		RedBlackTiledSmtm& operator = (const RedBlackTiledSmtm&) = delete;

		RedBlackTiledSmtm(RedBlackTiledSmtm&&) noexcept = delete;
		RedBlackTiledSmtm& operator = (RedBlackTiledSmtm&&) noexcept = delete;

	public:
		Handle create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config);
		SmartHandle createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config);

	public: // IResourceProvider
		bool valid(Handle handle) const override;
		void destroy(Handle handle) override;

		const DomainAabb2D& domain(Handle handle) const override;
		gl::Id texture(Handle handle) const override;

	public:
		void update();

		GLuint64 elapsed() const;
		f64 elapsedMean() const;

	private:
		uint m_workgroupSizeX{};
		uint m_workgroupSizeY{};

		gl::Id m_programSt0;
		gl::Id m_programSt1;
		Uniforms m_uniforms;
		TimeQuery m_querySt0;
		TimeQuery m_querySt1;

		Storage<DomainAabb2D> m_domainStorage;
		Storage<Solution>     m_solutionStorage;
		Storage<UpdateParams> m_configStorage;
	};
}
