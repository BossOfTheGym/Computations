#include "red_black_tiled.h"

#include <exception>

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include "dirichlet_util.h"

namespace dir2d
{
	// uniforms
	RedBlackTiled::Uniforms::Uniforms(gl::Id program)
	{
		setup(program);
		if (!valid()) {
			throw std::runtime_error("Failed to get locations from red-black-tiled program.");
		}
	}

	void RedBlackTiled::Uniforms::setup(gl::Id program)
	{
		curr = glGetUniformLocation(program, "curr");
		w    = glGetUniformLocation(program, "w");
		hx   = glGetUniformLocation(program, "hx");
		hy   = glGetUniformLocation(program, "hy");
	}

	bool RedBlackTiled::Uniforms::valid() const
	{
		return curr != -1 && w != -1 && hx != -1 && hy != -1;
	}


	// solution
	bool RedBlackTiled::Solution::create(Solution& solution, const DomainAabb2D& domain, const DataAabb2D& data)
	{
		i32 xVar = domain.xSplit + 1;
		i32 yVar = domain.ySplit + 1;

		for (int i = 0; i < 2; i++) {
			solution.s[i] = gl::create_texture(xVar, yVar, GL_R32F);
			glTextureSubImage2D(solution.s[i].id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.solution.get());
		}
		solution.f = gl::create_texture(xVar, yVar, GL_R32F);
		glTextureSubImage2D(solution.f.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.f.get());

		solution.curr = 0;
		solution.w = compute_optimal_w(domain.hx, domain.hy, domain.xSplit, domain.ySplit);

		return solution.s[0].valid() && solution.s[1].valid() && solution.f.valid();
	}

	gl::Id RedBlackTiled::Solution::texture() const
	{
		return s[curr].id;
	}

	void RedBlackTiled::Solution::pingpong()
	{
		curr ^= 1;
	}


	// method
	RedBlackTiled::RedBlackTiled(uint workgroupSizeX, uint workgroupSizeY, gl::Id program)
		: m_workgroupSizeX{workgroupSizeX}
		, m_workgroupSizeY{workgroupSizeY}
		, m_program{program}
		, m_uniforms(m_program)
	{}

	Handle RedBlackTiled::create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = acquire();

		Solution solution;
		if (!Solution::create(solution, domain, data)) {
			return null_handle;
		}

		m_domainStorage.emplace(handle, domain);
		m_solutionStorage.emplace(handle, std::move(solution));
		m_configStorage.emplace(handle, config);

		return handle;
	}

	SmartHandle RedBlackTiled::createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = create(domain, data, config);
		if (handle == null_handle) {
			return SmartHandle{};
		}
		return provideHandle(handle, this);
	}

	bool RedBlackTiled::valid(Handle handle) const
	{
		return m_domainStorage.has(handle); // can check only first	
	}

	void RedBlackTiled::destroy(Handle handle)
	{
		m_domainStorage.remove(handle);
		m_solutionStorage.remove(handle);
		m_configStorage.remove(handle);
	}

	const DomainAabb2D& RedBlackTiled::domain(Handle handle) const
	{
		return m_domainStorage.get(handle);
	}

	gl::Id RedBlackTiled::texture(Handle handle) const
	{
		return m_solutionStorage.get(handle).texture();
	}

	void RedBlackTiled::update()
	{
		constexpr int IMG0 = 0;
		constexpr int IMG1 = 1;
		constexpr int IMGF = 2;

		m_query.start();

		glUseProgram(m_program);
		for (auto& handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			glBindImageTexture(IMG0, solution.s[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG1, solution.s[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1f(m_uniforms.w, solution.w);
			glUniform1f(m_uniforms.hx, domain.hx);
			glUniform1f(m_uniforms.hy, domain.hy);

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			for (uint i = 0; i < config.itersPerUpdate; i++) {
				glUniform1i(m_uniforms.curr, solution.curr);
				glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
				solution.pingpong();
			}
		}

		m_query.end();
	}

	GLuint64 RedBlackTiled::elapsed() const
	{
		return m_query.elapsed();
	}

	f64 RedBlackTiled::elapsedMean() const
	{
		return m_query.elapsedMean();
	}
}
