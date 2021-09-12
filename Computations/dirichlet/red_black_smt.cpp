#include "red_black_smt.h"

#include <exception>

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include "dirichlet_util.h"

namespace dir2d
{
	// uniforms
	RedBlackTiledSmt::Uniforms::Uniforms(gl::Id program)
	{
		setup(program);
		if (!valid()) {
			throw std::runtime_error("Failed to get locations from red-black-tiled program.");
		}
	}

	void RedBlackTiledSmt::Uniforms::setup(gl::Id program)
	{
		curr = glGetUniformLocation(program, "curr");
		w    = glGetUniformLocation(program, "w");
		hx   = glGetUniformLocation(program, "hx");
		hy   = glGetUniformLocation(program, "hy");
		numWorkgroupsX = glGetUniformLocation(program, "numWorkgroupsX");
		numWorkgroupsY = glGetUniformLocation(program, "numWorkgroupsY");
	}

	bool RedBlackTiledSmt::Uniforms::valid() const
	{
		return curr != -1 && w != -1 
			&& hx != -1 && hy != -1 
			&& numWorkgroupsX != -1 && numWorkgroupsY != -1;
	}


	// solution
	bool RedBlackTiledSmt::Solution::create(
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

		solution.intermediate = gl::create_texture(xVar, yVar, GL_R32F);

		solution.f = gl::create_texture(xVar, yVar, GL_R32F);
		glTextureSubImage2D(solution.f.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.f.get());

		auto [workgroupsX, workgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, workgroupSizeX, workgroupSizeY);
		uint totalSize = sizeof(f32) * 4 * 2 * 2; // sizeof(f32) * total_corners * cell_x * cell_y
		solution.corners = create_work_buffer(workgroupsX, workgroupsY, totalSize);

		solution.curr = 0;
		solution.w = compute_optimal_w(domain.hx, domain.hy, domain.xSplit, domain.ySplit);

		return solution.s[0].valid() && solution.s[1].valid() 
			&& solution.intermediate.valid() 
			&& solution.f.valid()
			&& solution.corners.valid();
	}

	gl::Id RedBlackTiledSmt::Solution::texture() const
	{
		return s[curr].id;
	}

	void RedBlackTiledSmt::Solution::pingpong()
	{
		curr ^= 1;
	}


	// method
	RedBlackTiledSmt::RedBlackTiledSmt(uint workgroupSizeX, uint workgroupSizeY, gl::Id programSt0, gl::Id programSt1)
		: m_workgroupSizeX{workgroupSizeX}
		, m_workgroupSizeY{workgroupSizeY}
		, m_programSt0{programSt0}
		, m_programSt1{programSt1}
		, m_uniforms(m_programSt0)
	{
		Uniforms dummy(m_programSt1); // dummys check, no need for second uniforms struct 'cause uniform set is the same in both stages
	}

	Handle RedBlackTiledSmt::create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
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

	SmartHandle RedBlackTiledSmt::createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = create(domain, data, config);
		if (handle == null_handle) {
			return SmartHandle{};
		}
		return provideHandle(handle, this);
	}

	bool RedBlackTiledSmt::valid(Handle handle) const
	{
		return m_domainStorage.has(handle); // can check only first	
	}

	void RedBlackTiledSmt::destroy(Handle handle)
	{
		m_domainStorage.remove(handle);
		m_solutionStorage.remove(handle);
		m_configStorage.remove(handle);
	}

	const DomainAabb2D& RedBlackTiledSmt::domain(Handle handle) const
	{
		return m_domainStorage.get(handle);
	}

	gl::Id RedBlackTiledSmt::texture(Handle handle) const
	{
		return m_solutionStorage.get(handle).texture();
	}

	void RedBlackTiledSmt::update()
	{
		constexpr int IMG0 = 0;
		constexpr int IMG1 = 1;
		constexpr int IMG_INTERMEDIATE = 2;
		constexpr int IMGF = 3;
		constexpr int CORNERS = 0;

		// stage 0
		m_querySt0.start();

		glUseProgram(m_programSt0);
		for (auto& handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			if (config.itersPerUpdate == 0) {
				continue;
			}

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			auto stage0Workgroups = count_stage_workgroups(numWorkgroupsX, numWorkgroupsY, Stage::Stage0);

			glBindImageTexture(IMG0, solution.s[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG1, solution.s[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG_INTERMEDIATE, solution.intermediate.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CORNERS, solution.corners.id);

			glUniform1i(m_uniforms.curr, solution.curr);
			glUniform1f(m_uniforms.w, solution.w);
			glUniform1f(m_uniforms.hx, domain.hx);
			glUniform1f(m_uniforms.hy, domain.hy);
			glUniform1f(m_uniforms.numWorkgroupsX, numWorkgroupsX);
			glUniform1f(m_uniforms.numWorkgroupsY, numWorkgroupsY);

			glDispatchCompute(stage0Workgroups, 1, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		}

		m_querySt0.end();

		// stage1 
		m_querySt1.start();

		glUseProgram(m_programSt1);
		for (auto& handle : m_domainStorage) {
			auto& domain   = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config   = m_configStorage.get(handle);

			if (config.itersPerUpdate == 0) {
				continue;
			}

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			auto stage1Workgroups = count_stage_workgroups(numWorkgroupsX, numWorkgroupsY, Stage::Stage1);

			glBindImageTexture(IMG0, solution.s[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG1, solution.s[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMG_INTERMEDIATE, solution.intermediate.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CORNERS, solution.corners.id);

			glUniform1i(m_uniforms.curr, solution.curr);
			glUniform1f(m_uniforms.w, solution.w);
			glUniform1f(m_uniforms.hx, domain.hx);
			glUniform1f(m_uniforms.hy, domain.hy);
			glUniform1f(m_uniforms.numWorkgroupsX, numWorkgroupsX);
			glUniform1f(m_uniforms.numWorkgroupsY, numWorkgroupsY);

			glDispatchCompute(stage1Workgroups, 1, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
			solution.pingpong();
		}

		m_querySt1.end();
	}

	GLuint64 RedBlackTiledSmt::elapsed() const
	{
		return m_querySt0.elapsed() + m_querySt1.elapsed();
	}

	f64 RedBlackTiledSmt::elapsedMean() const
	{
		return m_querySt0.elapsedMean() + m_querySt1.elapsedMean();
	}
}
