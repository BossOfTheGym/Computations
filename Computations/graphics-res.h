#pragma once

#include <algorithm>

#include "gl-header.h"

namespace res
{
	using Id = GLuint;

	constexpr const Id null = 0u; 


	struct Shader
	{		
		Shader() = default;

		~Shader();

		Shader(Shader&& another);

		Shader& operator = (Shader&& another);

		Shader(const Shader&) = delete;

		Shader& operator = (const Shader&) = delete;


		void reset()
		{
			if (id != null)
			{
				glDeleteShader(id);

				id = null;
			}
		}

		bool valid() const
		{
			return id != null;
		}


		Id id{null};
	};

	struct ShaderProgram
	{
		ShaderProgram() = default;

		~ShaderProgram();

		ShaderProgram(ShaderProgram&& another);

		ShaderProgram& operator = (ShaderProgram&& another);

		ShaderProgram(const ShaderProgram&) = delete;

		ShaderProgram& operator = (const ShaderProgram&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Buffer
	{
		Buffer() = default;

		~Buffer();

		Buffer(Buffer&& another);

		Buffer& operator = (Buffer&& another);

		Buffer(const Buffer&) = delete;

		Buffer& operator = (const Buffer&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct VertexArray
	{
		VertexArray() = default;

		~VertexArray();

		VertexArray(VertexArray&& another);

		VertexArray& operator = (VertexArray&& another);

		VertexArray(const VertexArray&) = delete;

		VertexArray& operator = (const VertexArray&) = delete;


		void reset()
		{
			if (id != null)
			{
				glDeleteVertexArrays(1, &id);

				id = null;
			}
		}

		bool valid() const
		{
			return id != null;
		}


		Id id{null};
	};

	struct Texture
	{
		Texture() = default;

		~Texture();

		Texture(Texture&& another);

		Texture& operator = (Texture&& another);

		Texture(const Texture&) = delete;

		Texture& operator = (const Texture&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Query
	{
		Query() = default;

		Query(Query&& another) noexcept
		{
			*this = std::move(another);
		}

		~Query()
		{
			reset();
		}

		Query& operator = (Query&& another) noexcept
		{
			reset();

			id = another.id; another.id = null;

			return *this;
		}

		Query(const Query&) = delete;

		Query& operator = (const Query&) = delete;


		void reset()
		{
			if (id != null)
			{
				glDeleteQueries(1, &id);

				id = null;
			}
		}

		bool valid() const
		{
			return id != null;
		}


		Id id{null};
	};
}
