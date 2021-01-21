#pragma once

#include <algorithm>

// TODO : remove
#include "gl-header.h"


namespace res
{
	using Id = GLuint;

	constexpr const Id null = 0u; 


	struct Shader
	{		
		Shader() = default;

		~Shader()
		{
			reset();
		}

		Shader(Shader&& another) noexcept
		{
			*this = std::move(another);
		}

		Shader& operator = (Shader&& another) noexcept
		{
			reset();

			id = another.id; another.id = null;

			return *this;
		}

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

		~ShaderProgram()
		{
			reset();
		}

		ShaderProgram(ShaderProgram&& another) noexcept
		{
			*this = std::move(another);
		}

		ShaderProgram& operator = (ShaderProgram&& another) noexcept
		{
			reset();

			id = another.id; another.id = null;

			return *this;
		}

		ShaderProgram(const ShaderProgram&) = delete;

		ShaderProgram& operator = (const ShaderProgram&) = delete;


		void reset()
		{
			if (id != null)
			{
				glDeleteProgram(id);

				id = null;
			}
		}

		bool valid() const
		{
			return id != null;
		}


		Id id{null};
	};

	struct Buffer
	{
		Buffer() = default;

		~Buffer()
		{
			reset();
		}

		Buffer(Buffer&& another) noexcept
		{
			*this = std::move(another);
		}

		Buffer& operator = (Buffer&& another) noexcept
		{
			reset();

			id = another.id; another.id = null;

			return *this;
		}

		Buffer(const Buffer&) = delete;

		Buffer& operator = (const Buffer&) = delete;


		void reset()
		{
			if (id != null)
			{
				glDeleteBuffers(1, &id);

				id = null;
			}	
		}

		bool valid() const
		{
			return id != null;
		}


		Id id{null};
	};

	struct VertexArray
	{
		VertexArray() = default;

		~VertexArray()
		{
			reset();
		}

		VertexArray(VertexArray&& another) noexcept
		{
			*this = std::move(another);
		}

		VertexArray& operator = (VertexArray&& another) noexcept
		{
			reset();

			id = another.id; another.id = null;

			return *this;
		}

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

		~Texture()
		{
			reset();
		}

		Texture(Texture&& another) noexcept
		{
			*this = std::move(another);
		}

		Texture& operator = (Texture&& another) noexcept
		{
			reset();

			id = another.id; another.id = null;

			return *this;
		}

		Texture(const Texture&) = delete;

		Texture& operator = (const Texture&) = delete;


		void reset()
		{
			if (id != null)
			{
				glDeleteTextures(1, &id);

				id = null;
			}
		}

		bool valid() const
		{
			return id != null;
		}


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
