#include "jacoby.h"

#include <cassert>
#include <exception>

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include "dirichlet_util.h"

namespace dir2d
{
	// jacoby uniforms
	Jacoby::Uniforms::Uniforms(gl::Id program)
	{
		setup(program);
		if (!valid()) {
			throw std::runtime_error("Failed to get uniform locations from jacoby program.");
		}
	}

	void Jacoby::Uniforms::setup(gl::Id program)
	{
		curr = glGetUniformLocation(program, "curr");
		hx   = glGetUniformLocation(program, "hx");
		hy   = glGetUniformLocation(program, "hy");
	}

	bool Jacoby::Uniforms::valid() const
	{
		return curr != -1 && hx != -1 && hy != -1;
	}


	// solution data
	bool Jacoby::Solution::create(Solution& solution, const DomainAabb2D& domain, const DataAabb2D& data)
	{
		int xVars = domain.xSplit + 1;
		int yVars = domain.ySplit + 1;

		solution.curr = 0;
		for (int i = 0; i < 2; i++) {
			solution.s[i] = gl::create_texture(xVars, yVars, GL_R32F);
			glTextureSubImage2D(solution.s[i].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data.solution.get()); // boundary conditions
		}
		solution.f = gl::create_texture(xVars, yVars, GL_R32F);
		glTextureSubImage2D(solution.f.id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data.f.get());

		return solution.s[0].valid() && solution.s[1].valid() && solution.f.valid();
	}

	gl::Id Jacoby::Solution::texture() const
	{
		return s[curr].id;
	}

	void Jacoby::Solution::pingpong()
	{
		curr ^= 1;
	}


	// jacoby method
	Jacoby::Jacoby(uint workgroupSizeX, uint workgroupSizeY, gl::Id program)
		: m_workgroupSizeX{workgroupSizeX}
		, m_workgroupSizeY{workgroupSizeY}
		, m_program{program}
		, m_uniforms(m_program)
	{}

	Handle Jacoby::create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
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

	SmartHandle Jacoby::createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = create(domain, data, config);
		if (handle == null_handle) {
			return SmartHandle{};
		}
		return provideHandle(handle, this);
	}

	bool Jacoby::valid(Handle handle) const
	{
		return m_domainStorage.has(handle); // can check only first
	}

	void Jacoby::destroy(Handle handle)
	{
		m_domainStorage.remove(handle);
		m_solutionStorage.remove(handle);
		m_configStorage.remove(handle);
	}

	const DomainAabb2D& Jacoby::domain(Handle handle) const
	{
		return m_domainStorage.get(handle);
	}

	gl::Id Jacoby::texture(Handle handle) const
	{
		return m_solutionStorage.get(handle).texture();
	}

	void Jacoby::update()
	{
		constexpr int IMG0 = 0;
		constexpr int IMG1 = 1;
		constexpr int IMGF = 2;

		m_query.start();

		glUseProgram(m_program);
		for (auto handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			glBindImageTexture(IMG0, solution.s[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG1, solution.s[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1f(m_uniforms.hx, domain.hx);
			glUniform1f(m_uniforms.hy, domain.hy);

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			for (i32 i = 0; i < config.itersPerUpdate; i++) {
				glUniform1i(m_uniforms.curr, solution.curr);
				glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				solution.pingpong();
			}
		}

		m_query.end();
	}

	GLuint64 Jacoby::elapsed() const
	{
		return m_query.elapsed();
	}

	f64 Jacoby::elapsedMean() const
	{
		return m_query.elapsedMean();
	}
}