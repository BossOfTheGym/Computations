#pragma once

#include <cassert>

#include "core.h"
#include "app-system.h"
#include "graphics-res.h"
#include "graphics-res-util.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace app
{
	class App;

	// TODO : maybe must be part of graphics-res (part of buffer range)
	struct Requirements
	{
		static Requirements aligned(GLintptr baseOffset, GLsizeiptr size, GLint alignment)
		{
			auto rem = baseOffset % alignment;
			if (rem != 0)
			{
				baseOffset += alignment - rem;
			}
			return {baseOffset, size};
		}

		GLintptr last() const
		{
			return offset + size;
		}

		GLintptr offset{};
		GLsizei  size{};
	};


	struct PatchInfo
	{
		res::Id arrayId{res::null};
		i32 elementsBufferOffset{};
		i32 elements{};
	};

	// stores segmented quad, all it's vertices are from [0,1]x[0,1]
	class Patch
	{
	public:
		static const constexpr GLuint VERTEX_ATTRIB_INDEX = 0;
		static const constexpr GLuint VERTEX_BINDING_INDEX = 0;

		using Vertex = glm::vec2;
		using Index  = u16;

		static i32 vertices_count(u32 patchSizeX, u32 patchSizeY)
		{
			return patchSizeX * patchSizeY;
		}

		static i32 elements_count(u32 patchSizeX, u32 patchSizeY)
		{
			return 6 * (patchSizeX - 1) * (patchSizeY - 1);
		}

		static Requirements compute_vertices_requirements(GLintptr baseOffset, u32 patchSizeX, u32 patchSizeY)
		{
			return {baseOffset, sizeof(Vertex) * vertices_count(patchSizeX, patchSizeY)};
		}

		static Requirements compute_indices_requirements(GLintptr baseOffset, u32 patchSizeX, u32 patchSizeY)
		{
			return {baseOffset, sizeof(Index) * elements_count(patchSizeX, patchSizeY)};
		}

		static Requirements compute_patch_requirements(GLintptr baseOffset, u32 patchSizeX, u32 patchSizeY)
		{
			i32 verticesSize = sizeof(Vertex) * vertices_count(patchSizeX, patchSizeY);
			i32 elementsSize = sizeof(Index) * elements_count(patchSizeX, patchSizeY);

			return {baseOffset, verticesSize + elementsSize};
		} 


	public:
		Patch()
		{
			if (!try_create_vertex_array(m_patchArray))
			{
				std::cerr << "Failed to create vertex array for a Patch." << std::endl;

				assert(false);
			}
		}

		// buffer range must have enough storage for vertices and indices
		Patch(u32 patchSizeX, u32 patchSizeY, res::BufferRange&& bufferRange) : Patch()
		{
			reset(patchSizeX, patchSizeY, std::move(bufferRange));
		}


	public:
		// buffer range must have enough storage for vertices and indices
		void reset(u32 patchSizeX, u32 patchSizeY, res::BufferRange&& bufferRange)
		{
			if (patchSizeX == 0 || patchSizeY == 0 || !bufferRange.valid())
			{
				std::cerr << "Invalid parameters given to Patch." << std::endl;

				assert(false);
			}

			m_bufferRange = std::move(bufferRange);

			m_patchSizeX = patchSizeX;
			m_patchSizeY = patchSizeY;

			m_verticesReqs = compute_vertices_requirements(m_bufferRange.offset, patchSizeX, patchSizeY);
			m_indicesReqs  = compute_indices_requirements(m_verticesReqs.last(), patchSizeX, patchSizeY);

			auto vertices = createVertices();
			auto indices  = createIndices();

			storeVertexData(vertices, indices);

			setVertexAttributes();
		}

	private:
		std::unique_ptr<Vertex[]> createVertices() const
		{
			auto vertices = std::make_unique<Vertex[]>(m_patchSizeX * m_patchSizeY);

			f32 dx = 1.0 / (m_patchSizeX - 1);
			f32 dy = 1.0 / (m_patchSizeY - 1);

			auto vptr = vertices.get();
			for (i32 j = 0; j < m_patchSizeY; j++)
			{
				f32 y = j * dy;
				for (i32 i = 0; i < m_patchSizeX; i++)
				{
					f32 x = i * dx;

					*vptr++ = glm::vec2(x, y);
				}
			}

			return vertices;
		}

		std::unique_ptr<Index[]> createIndices() const
		{
			auto indices = std::make_unique<Index[]>(6 * (m_patchSizeX - 1) * (m_patchSizeY - 1));

			auto iptr = indices.get();
			for (i32 j = 0; j < m_patchSizeY - 1; j++)
			{
				for (i32 i = 0; i < m_patchSizeX - 1; i++)
				{
					*iptr++ = j * m_patchSizeX + i + m_patchSizeX + 1;
					*iptr++ = j * m_patchSizeX + i + m_patchSizeX;
					*iptr++ = j * m_patchSizeX + i;

					*iptr++ = j * m_patchSizeX + i + m_patchSizeX + 1;
					*iptr++ = j * m_patchSizeX + i;
					*iptr++ = j * m_patchSizeX + i + 1;
				}
			}

			return indices;
		}

		void storeVertexData(const std::unique_ptr<Vertex[]>& vertices, const std::unique_ptr<Index[]>& indices)
		{
			auto bufferRangePtr = (char*)glMapNamedBufferRange(m_bufferRange.bufferId, m_bufferRange.offset, m_bufferRange.size, GL_WRITE_ONLY);
			if (!bufferRangePtr)
			{
				std::cerr << "Failed to create mapping pointer to plot buffer" << std::endl;

				assert(false);
			}

			std::memcpy(bufferRangePtr + m_verticesReqs.offset - m_bufferRange.offset, vertices.get(), m_verticesReqs.size);
			std::memcpy(bufferRangePtr + m_indicesReqs.offset - m_bufferRange.offset, indices.get(), m_indicesReqs.size);

			glUnmapNamedBuffer(m_bufferRange.bufferId);
		}

		void setVertexAttributes()
		{
			glBindVertexArray(m_patchArray.id);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferRange.bufferId);
			glBindVertexBuffer(VERTEX_BINDING_INDEX, m_bufferRange.bufferId, m_verticesReqs.offset, sizeof(Vertex));
			glEnableVertexAttribArray(VERTEX_ATTRIB_INDEX);
			glVertexAttribFormat(VERTEX_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(VERTEX_ATTRIB_INDEX, VERTEX_BINDING_INDEX);
		}

	public:
		bool valid() const
		{
			return m_patchArray.valid() && m_bufferRange.valid();
		}


	public:
		PatchInfo info() const
		{
			return {m_patchArray.id, m_indicesReqs.offset, elements_count(m_patchSizeX, m_patchSizeY)};
		}


	private:
		res::VertexArray m_patchArray{};
		res::BufferRange m_bufferRange{};

		u32 m_patchSizeX{};
		u32 m_patchSizeY{};

		Requirements m_verticesReqs{};
		Requirements m_indicesReqs{};
	};

	// 
	class PlotUniformBuffer
	{
	public:
		using Mat4 = glm::mat4;
		using Vec3 = glm::vec3;

		static GLsizeiptr buffer_size()
		{
			return 4 * sizeof(Mat4) + sizeof(Vec3);
		}

		static Requirements compute_uniform_buffer_requirements(GLintptr baseOffset, GLint alignment)
		{			
			return Requirements::aligned(baseOffset, buffer_size(), alignment);
		}


	public:
		PlotUniformBuffer() = default;

		// buffer range must have enough space for uniform variables and must be aligned
		PlotUniformBuffer(res::BufferRange&& bufferRange)
		{
			reset(std::move(bufferRange));
		}


	public:
		// buffer range must have enough space for uniform variables and must be aligned
		void reset(res::BufferRange&& bufferRange)
		{
			m_bufferRange = std::move(bufferRange);
		}

		bool valid() const
		{
			return m_bufferRange.valid();
		}


	public:
		void setUniformData(const Mat4& proj, const Mat4& view, const Mat4& model, const Vec3& eye)
		{
			auto pvm = proj * view * model;

			auto ptr = (char*)glMapNamedBufferRange(m_bufferRange.bufferId, m_bufferRange.offset, m_bufferRange.size, GL_WRITE_ONLY);
			if (!ptr)
			{
				std::cerr << "Failed to map plot buffer for writing uniform." << std::endl;

				assert(false);
			}

			std::memcpy(ptr, glm::value_ptr(pvm)  , sizeof(Mat4)); ptr += sizeof(Mat4);
			std::memcpy(ptr, glm::value_ptr(proj) , sizeof(Mat4)); ptr += sizeof(Mat4);
			std::memcpy(ptr, glm::value_ptr(view) , sizeof(Mat4)); ptr += sizeof(Mat4);
			std::memcpy(ptr, glm::value_ptr(model), sizeof(Mat4)); ptr += sizeof(Mat4);
			std::memcpy(ptr, glm::value_ptr(eye)  , sizeof(Vec3));

			glUnmapNamedBuffer(m_bufferRange.bufferId);
		}


	private:
		res::BufferRange m_bufferRange{};
	};


	class PlotSystem : public System
	{
	public:
		PlotSystem(App& app, u32 patchSizeX, u32 patchSizeY, GLint uniformBufferAlignment) : System(app)
		{
			GLintptr baseOffset = 0;
			auto uniformBufferReqs = PlotUniformBuffer::compute_uniform_buffer_requirements(baseOffset, uniformBufferAlignment);
			auto patchReqs = Patch::compute_indices_requirements(uniformBufferReqs.last(), patchSizeX, patchSizeY);

			if (!try_create_storage_buffer(m_buffer, patchReqs.last(), GL_MAP_READ_BIT))
			{
				std::cerr << "Failed to create buffer for PlotSystem." << std::endl;

				assert(false);
			}

			m_plotUniformBuffer.reset(res::create_buffer_range(m_buffer, uniformBufferReqs.offset, uniformBufferReqs.size));
			m_patch.reset(patchSizeX, patchSizeY, res::create_buffer_range(m_buffer, patchReqs.offset, patchReqs.size));
		}

	public:
		void update()
		{
			// TODO
		}


	private:
		res::Buffer m_buffer;

		Patch             m_patch;
		PlotUniformBuffer m_plotUniformBuffer;

		glm::mat4 m_proj{};
		glm::mat4 m_view{};
		glm::mat4 m_model{};
		glm::vec3 m_eye{};
	};
}