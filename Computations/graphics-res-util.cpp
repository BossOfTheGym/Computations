#include "graphics-res-util.h"

#include <iostream>
#include <fstream>

namespace res
{
	// shaders
	std::string get_shader_info_log(const Shader& shader)
	{
		GLint length{};
		glGetShaderiv(shader.id, GL_INFO_LOG_LENGTH, &length);

		std::string infoLog; 
		infoLog.resize(length);
		glGetShaderInfoLog(shader.id, length, nullptr, infoLog.data());

		return infoLog;
	}

	Shader create_shader_from_source(GLenum shaderType, const std::string& source)
	{
		Shader shader{};

		shader.id = glCreateShader(shaderType);

		const char* src = source.c_str();
		GLsizei size = source.size();
		glShaderSource(shader.id, 1, &src, &size);

		glCompileShader(shader.id);

		GLint compileStatus{};
		glGetShaderiv(shader.id, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE)
		{
			std::cerr << "Failed to compile shader. Error log: " << std::endl;
			std::cerr << get_shader_info_log(shader) << std::endl;

			shader.reset();
		}
		return shader;
	}

	Shader create_shader_from_file(GLenum shaderType, const fs::path& path)
	{
		auto filePath = path.string();

		std::ifstream input(filePath);
		if (!input.is_open())
		{
			std::cerr << "Failed to open file " << std::quoted(filePath) << std::endl;

			return Shader{};
		}

		auto start  = std::istreambuf_iterator<char>(input);
		auto finish = std::istreambuf_iterator<char>();
		std::string contents(start, finish);

		return create_shader_from_source(shaderType, contents);
	}

	bool try_create_shader_from_source(Shader& shader, GLenum shaderType, const std::string& source)
	{
		shader = create_shader_from_source(shaderType, source);

		return shader.valid();
	}

	bool try_create_shader_from_file(Shader& shader, GLenum shaderType, const fs::path& file)
	{
		shader = create_shader_from_file(shaderType, file);

		return shader.valid();
	}


	// shader program
	std::string get_shader_program_info_log(const ShaderProgram& program)
	{
		GLint length{};
		glGetProgramiv(program.id, GL_INFO_LOG_LENGTH, &length);

		std::string infoLog;
		infoLog.resize(length);
		glGetProgramInfoLog(program.id, length, nullptr, infoLog.data());

		return infoLog;
	}


	// textures
	Texture create_texture(i32 width, i32 height, GLenum format)
	{
		Texture texture{};

		glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
		if (!texture.valid())
		{
			return Texture{};
		}

		glTextureStorage2D(texture.id, 1, format, width, height);
		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return texture;
	}

	Texture create_test_texture(i32 width, i32 height)
	{
		Texture texture = create_texture(width, height, GL_RGBA32F);
		if (!texture.valid())
		{
			return Texture{};
		}

		std::vector<f32> data(width * height * 4);

		const f32 PI = 3.14159265359;
		const f32 K = 2 * PI / 64;

		auto ptr = data.begin();
		for (i32 i = 0; i < height; i++)
		{
			for (i32 j = 0; j < width; j++)
			{
				f32 c = std::abs(std::sin(K * j) + std::sin(K * i));

				*ptr++ = c;
				*ptr++ = c;
				*ptr++ = c;
				*ptr++ = 1.0;
			}
		}

		glTextureSubImage2D(texture.id, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data.data());

		return texture;
	}

	bool try_create_texture(Texture& texture, i32 width, i32 height, GLenum format)
	{
		texture = create_texture(width, height, format);

		return texture.valid();
	}

	bool try_create_test_texture(Texture& texture, i32 width, i32 height)
	{
		texture = create_test_texture(width, height);

		return texture.valid();
	}


	// vertex array
	VertexArray create_vertex_array()
	{
		VertexArray array{};

		glCreateVertexArrays(1, &array.id);

		return array;
	}

	bool try_create_vertex_array(VertexArray& vertexArray)
	{
		vertexArray = create_vertex_array();

		return vertexArray.valid();
	}


	// buffer
	Buffer create_storage_buffer(GLsizeiptr size, GLbitfield usageFlags)
	{
		Buffer buffer{};

		glCreateBuffers(1, &buffer.id);
		if (!buffer.valid())
		{
			return Buffer{};
		}

		glNamedBufferStorage(buffer.id, size, nullptr, usageFlags);

		return buffer;
	}

	bool try_create_storage_buffer(Buffer& buffer, GLsizeiptr size, GLbitfield usageFlags)
	{
		buffer = create_storage_buffer(size, usageFlags);

		return buffer.valid();
	}


	// queries
	Query create_query()
	{
		Query query{};

		glGenQueries(1, &query.id);

		return query;
	}

	bool try_create_query(Query& query)
	{
		query = create_query();

		return query.valid();
	}
}