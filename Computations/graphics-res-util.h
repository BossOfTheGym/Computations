#pragma once

#include "core.h"
#include "graphics-res.h"

#include <string>
#include <iostream>
#include <filesystem>

namespace res
{
	namespace fs = std::filesystem;
	
	// shaders
	std::string get_shader_info_log(const Shader& shader);

	Shader create_shader_from_source(GLenum shaderType, const std::string& source);

	Shader create_shader_from_file(GLenum shaderType, const fs::path& path);

	bool try_create_shader_from_source(Shader& shader, GLenum shaderType, const std::string& source);

	bool try_create_shader_from_file(Shader& shader, GLenum shaderType, const fs::path& file);


	// shader program
	std::string get_shader_program_info_log(const ShaderProgram& program);

	template<class ... Shader>
	ShaderProgram create_shader_program(Shader&& ... shader)
	{
		ShaderProgram shaderProgram{};

		shaderProgram.id = glCreateProgram();
		(glAttachShader(shaderProgram.id, shader.id), ...);
		glLinkProgram(shaderProgram.id);
		(glDetachShader(shaderProgram.id, shader.id), ...);

		GLint linkStatus{};
		glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			std::cerr << "Failed to link shader program. Error log: " << std::endl;
			std::cerr << get_shader_program_info_log(shaderProgram) << std::endl;

			shaderProgram.reset();
		}

		return shaderProgram;
	}

	template<class ... Shader>
	bool try_create_shader_program(ShaderProgram& program, Shader&& ... shader)
	{
		program = create_shader_program(std::forward<Shader>(shader)...);

		return program.valid();
	}


	// textures
	Texture create_texture(i32 width, i32 height, GLenum format);

	Texture create_test_texture(i32 width, i32 height, i32 period);

	bool try_create_texture(Texture& texture, i32 width, i32 height, GLenum format);

	bool try_create_test_texture(Texture& texture, i32 width, i32 height, i32 period);


	// vertex array
	VertexArray create_vertex_array();

	bool try_create_vertex_array(VertexArray& vertexArray);


	// buffer
	Buffer create_storage_buffer(GLsizeiptr size, GLbitfield usageFlags);

	bool try_create_storage_buffer(Buffer& buffer, GLsizeiptr size, GLbitfield usageFlags);


	// queries
	Query create_query();

	bool try_create_query(Query& query);
}
