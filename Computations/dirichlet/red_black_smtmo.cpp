#include "red_black_smtmo.h"

#include <exception>

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include "dirichlet_util.h"

namespace dir2d
{
	// uniforms
	RedBlackTiledSmtmo::Uniforms::Uniforms(gl::Id program)
	{
		setup(program);
		if (!valid()) {
			throw std::runtime_error("Failed to get locations from red-black-tiled program.");
		}
	}

	void RedBlackTiledSmtmo::Uniforms::setup(gl::Id program)
	{
		curr = glGetUniformLocation(program, "curr");
		stage = glGetUniformLocation(program, "stage");
		w    = glGetUniformLocation(program, "w");
		hx   = glGetUniformLocation(program, "hx");
		hy   = glGetUniformLocation(program, "hy");
		numWorkgroupsX = glGetUniformLocation(program, "numWorkgroupsX");
		numWorkgroupsY = glGetUniformLocation(program, "numWorkgroupsY");
	}

	bool RedBlackTiledSmtmo::Uniforms::valid() const
	{
		return curr != -1 
			&& stage != -1
			&& w != -1
			&& hx != -1 
			&& hy != -1
			&& numWorkgroupsX != -1 
			&& numWorkgroupsY != -1;
	}


	// solution
	bool RedBlackTiledSmtmo::Solution::create(
		Solution& solution,
		const DomainAabb2D& domain,
		const DataAabb2D& data,
		uint workgroupSizeX,
		uint workgroupSizeY)
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
		solution.stage = 0;
		solution.w = compute_optimal_w(domain.hx, domain.hy, domain.xSplit, domain.ySplit);

		return solution.s[0].valid()
			&& solution.s[1].valid()
			&& solution.f.valid();
	}

	gl::Id RedBlackTiledSmtmo::Solution::texture() const
	{
		return s[curr].id;
	}

	void RedBlackTiledSmtmo::Solution::pingpong()
	{
		curr ^= 1;
		stage ^= 1; // TODO : test with and without stage pingponging
	}


	// method
	RedBlackTiledSmtmo::RedBlackTiledSmtmo(uint workgroupSizeX, uint workgroupSizeY, gl::Id programSt0, gl::Id programSt1)
		: m_workgroupSizeX{workgroupSizeX}
		, m_workgroupSizeY{workgroupSizeY}
		, m_programSt0{programSt0}
		, m_programSt1{programSt1}
		, m_uniformsSt0(m_programSt0)
		, m_uniformsSt1(m_programSt1)
	{
		Uniforms dummy(m_programSt1); // dummies check, no need for second uniforms struct 'cause uniform set is the same in both stages
	}

	Handle RedBlackTiledSmtmo::create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = acquire();

		Solution solution;
		if (!Solution::create(solution, domain, data, m_workgroupSizeX, m_workgroupSizeY)) {
			return null_handle;
		}

		m_domainStorage.emplace(handle, domain);
		m_solutionStorage.emplace(handle, std::move(solution));
		m_configStorage.emplace(handle, config);

		return handle;
	}

	SmartHandle RedBlackTiledSmtmo::createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = create(domain, data, config);
		if (handle == null_handle) {
			return SmartHandle{};
		}
		return provideHandle(handle, this);
	}

	bool RedBlackTiledSmtmo::valid(Handle handle) const
	{
		return m_domainStorage.has(handle); // can check only first	
	}

	void RedBlackTiledSmtmo::destroy(Handle handle)
	{
		m_domainStorage.remove(handle);
		m_solutionStorage.remove(handle);
		m_configStorage.remove(handle);
	}

	const DomainAabb2D& RedBlackTiledSmtmo::domain(Handle handle) const
	{
		return m_domainStorage.get(handle);
	}

	gl::Id RedBlackTiledSmtmo::texture(Handle handle) const
	{
		return m_solutionStorage.get(handle).texture();
	}

	void RedBlackTiledSmtmo::update()
	{
		constexpr int IMG0 = 0;
		constexpr int IMG1 = 1;
		constexpr int IMGF = 2;

		// stage 0
		glUseProgram(m_programSt0);

		m_querySt0.start();
		for (auto& handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			if (config.itersPerUpdate == 0) {
				continue;
			}

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			auto stage0Workgroups = count_stage_workgroups(numWorkgroupsX, numWorkgroupsY, Stage{solution.stage});

			glBindImageTexture(IMG0, solution.s[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG1, solution.s[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1i(m_uniformsSt0.curr, solution.curr);
			glUniform1i(m_uniformsSt0.stage, solution.stage);
			glUniform1f(m_uniformsSt0.w, solution.w);
			glUniform1f(m_uniformsSt0.hx, domain.hx);
			glUniform1f(m_uniformsSt0.hy, domain.hy);
			glUniform1i(m_uniformsSt0.numWorkgroupsX, numWorkgroupsX);
			glUniform1i(m_uniformsSt0.numWorkgroupsY, numWorkgroupsY);

			glDispatchCompute(stage0Workgroups, 1, 1);
		}
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		m_querySt0.end();


		// stage1 
		glUseProgram(m_programSt1);

		m_querySt1.start();
		for (auto& handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			if (config.itersPerUpdate == 0) {
				continue;
			}

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			auto stage1Workgroups = count_stage_workgroups(numWorkgroupsX, numWorkgroupsY, Stage{solution.stage ^ 1});

			glBindImageTexture(IMG0, solution.s[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG1, solution.s[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1i(m_uniformsSt1.curr, solution.curr);
			glUniform1i(m_uniformsSt1.stage, solution.stage ^ 1);
			glUniform1f(m_uniformsSt1.w, solution.w);
			glUniform1f(m_uniformsSt1.hx, domain.hx);
			glUniform1f(m_uniformsSt1.hy, domain.hy);
			glUniform1i(m_uniformsSt1.numWorkgroupsX, numWorkgroupsX);
			glUniform1i(m_uniformsSt1.numWorkgroupsY, numWorkgroupsY);

			glDispatchCompute(stage1Workgroups, 1, 1);
			solution.pingpong();
		}
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		m_querySt1.end();
	}

	GLuint64 RedBlackTiledSmtmo::elapsed() const
	{
		return m_querySt0.elapsed() + m_querySt1.elapsed();
	}

	f64 RedBlackTiledSmtmo::elapsedMean() const
	{
		return m_querySt0.elapsedMean() + m_querySt1.elapsedMean();
	}
}
