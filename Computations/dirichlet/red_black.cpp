#include "red_black.h"

#include <exception>

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include "dirichlet_util.h"

namespace dir2d
{
	// uniforms
	RedBlack::Uniforms::Uniforms(gl::Id program)
	{
		setup(program);
		if (!valid()) {
			throw std::runtime_error("Failed to get uniform locations from red-black program.");
		}
	}

	void RedBlack::Uniforms::setup(gl::Id program)
	{
		rb = glGetUniformLocation(program, "rb");
		w  = glGetUniformLocation(program, "w");
		hx = glGetUniformLocation(program, "hx");
		hy = glGetUniformLocation(program, "hy");
	}

	bool RedBlack::Uniforms::valid() const
	{
		return rb != -1 && w != -1 && hx != -1 && hy != -1;
	}


	// data
	bool RedBlack::Solution::create(Solution& solution, const DomainAabb2D& domain, const DataAabb2D& data)
	{
		i32 xVar = domain.xSplit + 1;
		i32 yVar = domain.ySplit + 1;

		solution.s = gl::create_texture(xVar, yVar, GL_R32F);
		glTextureSubImage2D(solution.s.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.solution.get());

		solution.f = gl::create_texture(xVar, yVar, GL_R32F);
		glTextureSubImage2D(solution.f.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.f.get());

		solution.w = compute_optimal_w(domain.hx, domain.hy, domain.xSplit, domain.ySplit);

		return solution.s.valid() && solution.f.valid();
	}

	gl::Id RedBlack::Solution::texture() const
	{
		return s.id;
	}


	// red-black method
	RedBlack::RedBlack(uint workgroupSizeX, uint workgroupSizeY, gl::ShaderProgram&& program)
		: m_workgroupSizeX{workgroupSizeX}
		, m_workgroupSizeY{workgroupSizeY}
		, m_program{std::move(program)}
		, m_uniforms(m_program.id)
	{}

	Handle RedBlack::create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = acquire();

		Solution solution;
		if (!Solution::create(solution, domain, data)) {
			return gl::null;
		}

		m_domainStorage.emplace(handle, domain);
		m_solutionStorage.emplace(handle, std::move(solution));
		m_configStorage.emplace(handle, config);

		return handle;
	}

	SmartHandle RedBlack::createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = create(domain, data, config);
		if (handle == null_handle) {
			return SmartHandle{};
		}
		return provideHandle(handle, this);
	}

	bool RedBlack::valid(Handle handle) const
	{
		return m_domainStorage.has(handle); // can check only first	
	}

	void RedBlack::destroy(Handle handle)
	{
		m_domainStorage.remove(handle);
		m_solutionStorage.remove(handle);
		m_configStorage.remove(handle);
	}

	const DomainAabb2D& RedBlack::domain(Handle handle) const
	{
		return m_domainStorage.get(handle);
	}

	gl::Id RedBlack::texture(Handle handle) const
	{
		return m_solutionStorage.get(handle).s.id;
	}

	void RedBlack::update()
	{
		constexpr int IMG = 0;
		constexpr int IMGF = 1;

		m_query.start();

		glUseProgram(m_program.id);

		for (auto& handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			glBindImageTexture(IMG, solution.s.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1f(m_uniforms.w, solution.w);
			glUniform1f(m_uniforms.hx, domain.hx);
			glUniform1f(m_uniforms.hy, domain.hy);

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			for (i32 i = 0; i < config.itersPerUpdate; i++) {
				glUniform1i(m_uniforms.rb, 0);
				glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				glUniform1i(m_uniforms.rb, 1);
				glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
			}
		}

		m_query.end();
	}

	GLuint64 RedBlack::elapsed() const
	{
		return m_query.elapsed();
	}

	f64 RedBlack::elapsedMean() const
	{
		return m_query.elapsedMean();
	}
}