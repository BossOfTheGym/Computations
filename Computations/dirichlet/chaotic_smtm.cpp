#include "chaotic_smtm.h"

#include <exception>

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include "dirichlet_util.h"

namespace dir2d
{
	// uniforms
	ChaoticSmtm::Uniforms::Uniforms(gl::Id program)
	{
		setup(program);
		if (!valid())
		{
			throw std::runtime_error("Failed to get locations from red-black-tiled program.");
		}
	}

	void ChaoticSmtm::Uniforms::setup(gl::Id program)
	{
		hx = glGetUniformLocation(program, "hx");
		hy = glGetUniformLocation(program, "hy");
		numWorkgroupsX = glGetUniformLocation(program, "numWorkgroupsX");
		numWorkgroupsY = glGetUniformLocation(program, "numWorkgroupsY");
	}

	bool ChaoticSmtm::Uniforms::valid() const
	{
		return hx != -1 && hy != -1
			&& numWorkgroupsX != -1 && numWorkgroupsY != -1;
	}


	// solution
	bool ChaoticSmtm::Solution::create(
		Solution& solution,
		const DomainAabb2D& domain,
		const DataAabb2D& data,
		uint workgroupSizeX,
		uint workgroupSizeY)
	{
		i32 xVar = domain.xSplit + 1;
		i32 yVar = domain.ySplit + 1;

		solution.s = gl::create_texture(xVar, yVar, GL_R32F);
		glTextureSubImage2D(solution.s.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.solution.get());

		solution.f = gl::create_texture(xVar, yVar, GL_R32F);
		glTextureSubImage2D(solution.f.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data.f.get());

		return solution.s.valid()
			&& solution.f.valid();
	}

	gl::Id ChaoticSmtm::Solution::texture() const
	{
		return s.id;
	}


	// method
	ChaoticSmtm::ChaoticSmtm(uint workgroupSizeX, uint workgroupSizeY, gl::Id programSt0, gl::Id programSt1)
		: m_workgroupSizeX{workgroupSizeX}
		, m_workgroupSizeY{workgroupSizeY}
		, m_programSt0{programSt0}
		, m_programSt1{programSt1}
		, m_uniformsSt0(m_programSt0)
		, m_uniformsSt1(m_programSt1)
	{
		Uniforms dummy(m_programSt1); // dummys check, no need for second uniforms struct 'cause uniform set is the same in both stages
	}

	Handle ChaoticSmtm::create(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = acquire();

		Solution solution;
		if (!Solution::create(solution, domain, data, m_workgroupSizeX, m_workgroupSizeY))
		{
			return null_handle;
		}

		m_domainStorage.emplace(handle, domain);
		m_solutionStorage.emplace(handle, std::move(solution));
		m_configStorage.emplace(handle, config);

		return handle;
	}

	SmartHandle ChaoticSmtm::createSmart(const DomainAabb2D& domain, const DataAabb2D& data, const UpdateParams& config)
	{
		Handle handle = create(domain, data, config);
		if (handle == null_handle)
		{
			return SmartHandle{};
		}
		return provideHandle(handle, this);
	}

	bool ChaoticSmtm::valid(Handle handle) const
	{
		return m_domainStorage.has(handle); // can check only first	
	}

	void ChaoticSmtm::destroy(Handle handle)
	{
		m_domainStorage.remove(handle);
		m_solutionStorage.remove(handle);
		m_configStorage.remove(handle);
	}

	const DomainAabb2D& ChaoticSmtm::domain(Handle handle) const
	{
		return m_domainStorage.get(handle);
	}

	gl::Id ChaoticSmtm::texture(Handle handle) const
	{
		return m_solutionStorage.get(handle).texture();
	}

	void ChaoticSmtm::update()
	{
		constexpr int IMGS = 0;
		constexpr int IMGF = 2;

		// stage 0
		glUseProgram(m_programSt0);

		m_querySt0.start();
		for (auto& handle : m_domainStorage) {
			auto& domain = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config = m_configStorage.get(handle);

			if (config.itersPerUpdate == 0) {
				continue;
			}

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			auto stage0Workgroups = count_stage_workgroups(numWorkgroupsX, numWorkgroupsY, Stage::Stage0);

			glBindImageTexture(IMGS, solution.s.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1f(m_uniformsSt0.hx, domain.hx);
			glUniform1f(m_uniformsSt0.hy, domain.hy);
			glUniform1i(m_uniformsSt0.numWorkgroupsX, numWorkgroupsX);
			glUniform1i(m_uniformsSt0.numWorkgroupsY, numWorkgroupsY);

			glDispatchCompute(stage0Workgroups, 1, 1);
		}
		// TODO : test with and without barrier
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		m_querySt0.end();


		// stage1 
		glUseProgram(m_programSt1);

		m_querySt1.start();
		for (auto& handle : m_domainStorage) {
			auto& domain = m_domainStorage.get(handle);
			auto& solution = m_solutionStorage.get(handle);
			auto& config = m_configStorage.get(handle);

			if (config.itersPerUpdate == 0) {
				continue;
			}

			auto [numWorkgroupsX, numWorkgroupsY] = get_num_workgroups(domain.xSplit, domain.ySplit, m_workgroupSizeX, m_workgroupSizeY);
			auto stage1Workgroups = count_stage_workgroups(numWorkgroupsX, numWorkgroupsY, Stage::Stage1);

			glBindImageTexture(IMGS, solution.s.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			glBindImageTexture(IMGF, solution.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glUniform1f(m_uniformsSt1.hx, domain.hx);
			glUniform1f(m_uniformsSt1.hy, domain.hy);
			glUniform1i(m_uniformsSt1.numWorkgroupsX, numWorkgroupsX);
			glUniform1i(m_uniformsSt1.numWorkgroupsY, numWorkgroupsY);

			glDispatchCompute(stage1Workgroups, 1, 1);
		}
		// TODO : test with and without barrier
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		m_querySt1.end();
	}

	GLuint64 ChaoticSmtm::elapsed() const
	{
		return m_querySt0.elapsed() + m_querySt1.elapsed();
	}

	f64 ChaoticSmtm::elapsedMean() const
	{
		return m_querySt0.elapsedMean() + m_querySt1.elapsedMean();
	}
}
