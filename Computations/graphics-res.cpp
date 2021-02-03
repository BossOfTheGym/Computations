#include "graphics-res.h"


namespace res
{
	// Shader
	Shader::~Shader()
	{
		reset();
	}

	Shader::Shader(Shader&& another) noexcept
	{
		*this = std::move(another);
	}

	Shader& Shader::operator = (Shader&& another) noexcept
	{
		reset();

		id = another.id; another.id = null;

		return *this;
	}

	void Shader::reset()
	{
		if (id != null)
		{
			glDeleteShader(id);

			id = null;
		}
	}

	bool Shader::valid() const
	{
		return id != null;
	}


	// ShaderProgram
	ShaderProgram::~ShaderProgram()
	{
		reset();
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& another) noexcept
	{
		*this = std::move(another);
	}

	ShaderProgram& ShaderProgram::operator = (ShaderProgram&& another) noexcept
	{
		reset();

		id = another.id; another.id = null;

		return *this;
	}

	void ShaderProgram::reset()
	{
		if (id != null)
		{
			glDeleteProgram(id);

			id = null;
		}
	}

	bool ShaderProgram::valid() const
	{
		return id != null;
	}


	// Buffer
	Buffer::~Buffer()
	{
		reset();
	}

	Buffer::Buffer(Buffer&& another) noexcept
	{
		*this = std::move(another);
	}

	Buffer& Buffer::operator = (Buffer&& another) noexcept
	{
		reset();

		id = another.id; another.id = null;

		return *this;
	}

	void Buffer::reset()
	{
		if (id != null)
		{
			glDeleteBuffers(1, &id);

			id = null;
		}	
	}

	bool Buffer::valid() const
	{
		return id != null;
	}


	// VertexArray
	VertexArray::~VertexArray()
	{
		reset();
	}

	VertexArray::VertexArray(VertexArray&& another) noexcept
	{
		*this = std::move(another);
	}

	VertexArray& VertexArray::operator = (VertexArray&& another) noexcept
	{
		reset();

		id = another.id; another.id = null;

		return *this;
	}

	void VertexArray::reset()
	{
		if (id != null)
		{
			glDeleteVertexArrays(1, &id);

			id = null;
		}
	}

	bool VertexArray::valid() const
	{
		return id != null;
	}


	// Texture
	Texture::~Texture()
	{
		reset();
	}

	Texture::Texture(Texture&& another) noexcept
	{
		*this = std::move(another);
	}

	Texture& Texture::operator = (Texture&& another) noexcept
	{
		reset();

		id = another.id; another.id = null;

		return *this;
	}

	void Texture::reset()
	{
		if (id != null)
		{
			glDeleteTextures(1, &id);

			id = null;
		}
	}

	bool Texture::valid() const
	{
		return id != null;
	}


	// Query
	Query::Query(Query&& another) noexcept
	{
		*this = std::move(another);
	}

	Query::~Query()
	{
		reset();
	}

	Query& Query::operator = (Query&& another) noexcept
	{
		reset();

		id = another.id; another.id = null;

		return *this;
	}

	void Query::reset()
	{
		if (id != null)
		{
			glDeleteQueries(1, &id);

			id = null;
		}
	}

	bool Query::valid() const
	{
		return id != null;
	}
}