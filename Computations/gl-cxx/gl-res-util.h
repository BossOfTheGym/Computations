#pragma once

#include <core.h>

#include "gl-res.h"

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <type_traits>


namespace gl
{
	// TODO : remove error output from functions/ It must be queried separately
	// TODO : return error status

	namespace fs = std::filesystem;
	
	// shaders
	GLenum shader_type_from_extension(const fs::path& file);

	std::string get_shader_info_log(const Shader& shader);

	Shader create_shader_from_source(GLenum shaderType, const std::string& source);

	Shader create_shader_from_file(GLenum shaderType, const fs::path& path);

	// shader program
	std::string get_shader_program_info_log(const ShaderProgram& program);

	bool check_program_link_status(const ShaderProgram& program);

	ShaderProgram create_shader_program(uint count, ...);

	ShaderProgram create_shader_program(uint count, Id* shaderIds);

	template<class ... shader_t, std::enable_if_t<(std::is_same_v<std::remove_reference_t<std::remove_cv_t<shader_t>>, Shader> && ...), int> = 0>
	ShaderProgram create_shader_program(shader_t&& ... shader)
	{
		return create_shader_program(sizeof...(shader_t), shader.id...);
	}

	// textures
	Texture create_texture();

	Texture create_texture(uint width, uint height, GLenum format);

	Texture create_test_texture(uint width, uint height, uint period);

	Texture create_stencil_texture(uint width, uint height);

	// vertex array
	VertexArray create_vertex_array();

	// buffer
	Buffer create_storage_buffer(GLsizeiptr size, GLbitfield usageFlags, void* data = nullptr);


	// query
	Query create_query();


	// framebuffer
	Framebuffer create_framebuffer();


	// fence
	FenceSync create_fence_sync();


	// buffer range
	BufferRange create_buffer_range(const Buffer& buffer, GLintptr offset, GLsizei size);
}
