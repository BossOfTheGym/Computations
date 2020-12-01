#pragma once

#include <algorithm>

#include "gl-header.h"


namespace res
{
	using Id = GLuint;

	constexpr const Id null = 0; 


	// TODO : this looks pretty bloaty
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
		Shader(Id resource = null) : Resource(resource)
		{}

		Shader(Shader&& another) noexcept : Resource(std::move(another)) 
		{}

		~Shader()
		{
			if (id != null)
			{
				glDeleteShader(id);
			}
		}

		Shader& operator = (Shader&& another) noexcept
		{
			static_cast<Resource&>(*this) = std::move(another);

			return *this;
		}
	};

	struct ShaderProgram : Resource
	{
		ShaderProgram(Id resource = null) : Resource(id)
		{}

		ShaderProgram(ShaderProgram&& another) noexcept : Resource(std::move(another))
		{}

		~ShaderProgram()
		{
			if (id != null)
			{
				glDeleteProgram(id);
			}
		}

		ShaderProgram& operator = (ShaderProgram&& another) noexcept
		{
			static_cast<Resource&>(*this) =  std::move(another);

			return *this;
		}
	};

	struct Buffer : Resource
	{
		Buffer(Id resource = null) : Resource(id)
		{}

		Buffer(Buffer&& another) noexcept : Resource(std::move(another))
		{}

		~Buffer()
		{
			if (id != null)
			{
				glDeleteBuffers(1, &id);
			}
		}

		Buffer& operator = (Buffer&& another) noexcept
		{
			static_cast<Resource&>(*this) =  std::move(another);

			return *this;
		}
	};

	struct VertexArray : Resource
	{
		VertexArray(Id resource = null) : Resource(id)
		{}

		VertexArray(VertexArray&& another) noexcept : Resource(std::move(another))
		{}

		~VertexArray()
		{
			if (id != null)
			{
				glDeleteVertexArrays(1, &id);
			}
		}

		VertexArray& operator = (VertexArray&& another) noexcept
		{
			static_cast<Resource&>(*this) =  std::move(another);

			return *this;
		}
	};

	struct Texture : Resource
	{
		Texture(Id resource = null) : Resource(id)
		{}

		Texture(Texture&& another) noexcept : Resource(std::move(another))
		{}

		~Texture()
		{
			if (id != null)
			{
				glDeleteTextures(1, &id);
			}
		}

		Texture& operator = (Texture&& another) noexcept
		{
			static_cast<Resource&>(*this) =  std::move(another);

			return *this;
		}
	};
}
