#pragma once

#include <algorithm>

#include "gl-header.h"


namespace res
{
	using Id = GLuint;

	constexpr const Id null = 0; 


	struct Resource
	{
		Resource(Id resource = null) : id{resource}
		{}

		Resource(Resource&& another) noexcept
		{
			*this = std::move(another);
		}

		Resource(const Resource&) = delete;

		~Resource()
		{
			id = null;
		}

		Resource& operator = (Resource&& another) noexcept
		{
			std::swap(id, another.id);

			return *this;
		}

		Resource& operator = (const Resource&) = delete;


		Id id{};
	};


	struct Shader : Resource
	{		
		Shader(Shader&&) noexcept = default;

		~Shader()
		{
			if (id != null)
			{
				glDeleteShader(id);
			}
		}

		Shader& operator = (Shader&&) noexcept = default;
	};

	struct ShaderProgram : Resource
	{
		ShaderProgram(ShaderProgram&&) noexcept = default;

		~ShaderProgram()
		{
			if (id != null)
			{
				glDeleteProgram(id);
			}
		}

		ShaderProgram& operator = (ShaderProgram&&) noexcept = default;
	};

	struct Buffer : Resource
	{
		Buffer(Buffer&&) = default;

		~Buffer()
		{
			if (id != null)
			{
				glDeleteBuffers(1, &id);
			}
		}

		Buffer& operator = (Buffer&& another) noexcept = default;
	};

	struct VertexArray : Resource
	{
		VertexArray(VertexArray&&) = default;

		~VertexArray()
		{
			if (id != null)
			{
				glDeleteVertexArrays(1, &id);
			}
		}

		VertexArray& operator = (VertexArray&& another) noexcept = default;
	};

	struct Texture : Resource
	{
		Texture(Texture&&) = default;

		~Texture()
		{
			if (id != null)
			{
				glDeleteTextures(1, &id);
			}
		}

		Texture& operator = (Texture&& another) noexcept = default;
	};

	struct Query : Resource
	{
		Query(Query&&) noexcept = default;

		~Query()
		{
			if (id != null)
			{
				glDeleteQueries(1, &id);
			}
		}

		Query& operator = (Query&&) noexcept = default;
	};
}
