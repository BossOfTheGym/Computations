#include "gl-res-util.h"

#include "gl-header.h"

#include <map>
#include <string>
#include <fstream>
#include <cstdarg>

namespace
{
	std::map<std::string, GLenum> EXT_TO_SHADER_TYPE = 
	{
		{".vert", GL_VERTEX_SHADER}, 
		{".tesc", GL_TESS_CONTROL_SHADER},
		{".tese", GL_TESS_EVALUATION_SHADER},
		{".geom", GL_GEOMETRY_SHADER},
		{".frag", GL_FRAGMENT_SHADER},
		{".comp", GL_COMPUTE_SHADER}
	};
}

namespace gl
{
	// shaders
	GLenum shader_type_from_extension(const fs::path& file)
	{
		auto ext = file.extension().string();
		if (auto it = EXT_TO_SHADER_TYPE.find(ext); it != EXT_TO_SHADER_TYPE.end()) {
			return it->second;
		}
		return -1;
	}

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
		if (!shader.valid()) {
			return Shader{};
		}

		const char* src = source.c_str();
		GLsizei size = source.size();
		glShaderSource(shader.id, 1, &src, &size);

		glCompileShader(shader.id);

		GLint compileStatus{};
		glGetShaderiv(shader.id, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE) {
			shader.reset();
		}

		return shader;
	}

	Shader create_shader_from_file(GLenum shaderType, const fs::path& path)
	{
		auto filePath = path.string();

		std::ifstream input(filePath);
		if (!input.is_open()) {
			return Shader{};
		}

		auto start  = std::istreambuf_iterator<char>(input);
		auto finish = std::istreambuf_iterator<char>();
		std::string contents(start, finish);

		return create_shader_from_source(shaderType, contents);
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

	bool check_program_link_status(const ShaderProgram& prog)
	{
		GLint linkStatus{};
		glGetProgramiv(prog.id, GL_LINK_STATUS, &linkStatus);
		return linkStatus == GL_TRUE;
	}

	ShaderProgram create_shader_program(uint count, ...)
	{
		ShaderProgram shaderProgram{};

		shaderProgram.id = glCreateProgram();
		if (!shaderProgram.valid()) {
			return ShaderProgram{};
		}

		va_list shaderIds;

		va_start(shaderIds, count);
		for (uint i = 0; i < count; i++) {
			Id shaderId = va_arg(shaderIds, Id);
			glAttachShader(shaderProgram.id, shaderId);
		}
		va_end(shaderIds);

		glLinkProgram(shaderProgram.id);

		va_start(shaderIds, count);
		for (uint i = 0; i < count; i++) {
			Id shaderId = va_arg(shaderIds, Id);
			glDetachShader(shaderProgram.id, shaderId);
		}
		va_end(shaderIds);

		GLint linkStatus{};
		glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			shaderProgram.reset();
		}

		return shaderProgram;
	}

	ShaderProgram create_shader_program(uint count, Id* shaderIds)
	{
		ShaderProgram shaderProgram{};

		shaderProgram.id = glCreateProgram();
		if (!shaderProgram.valid()) {
			return ShaderProgram{};
		}

		for (i32 i = 0; i < count; i++) {
			glAttachShader(shaderProgram.id, shaderIds[i]);
		}

		glLinkProgram(shaderProgram.id);

		for (i32 i = 0; i < count; i++) {
			glDetachShader(shaderProgram.id, shaderIds[i]);
		}

		GLint linkStatus{};
		glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			shaderProgram.reset();
		}

		return shaderProgram;
	}


	// textures
	Texture create_texture()
	{
		Texture texture{};

		glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
		if (!texture.valid()) {
			return Texture{};
		}

		return texture;
	}

	Texture create_texture(i32 width, i32 height, GLenum format)
	{
		Texture texture{};

		glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
		if (!texture.valid()) {
			return Texture{};
		}

		glTextureStorage2D(texture.id, 1, format, width, height);
		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return texture;
	}

	Texture create_test_texture(uint width, uint height, uint period)
	{
		Texture texture = create_texture(width, height, GL_RGBA32F);
		if (!texture.valid()) {
			return Texture{};
		}

		std::vector<f32> data(width * height * 4);

		const f32 PI = 3.14159265359;
		const f32 K = 2 * PI / period;

		auto ptr = data.begin();
		for (uint i = 0; i < height; i++) {
			for (uint j = 0; j < width; j++) {
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

	Texture create_stencil_texture(uint width, uint height)
	{
		Texture texture;

		glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
		if (!texture.valid()) {
			return Texture{};
		}

		glTextureStorage2D(texture.id, 1, GL_STENCIL_INDEX8, width, height);
		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return texture;
	}


	// vertex array
	VertexArray create_vertex_array()
	{
		VertexArray array{};

		glCreateVertexArrays(1, &array.id);

		return array;
	}


	// buffer
	Buffer create_storage_buffer(GLsizeiptr size, GLbitfield usageFlags, void* data)
	{
		Buffer buffer{};

		glCreateBuffers(1, &buffer.id);
		if (!buffer.valid()) {
			return Buffer{};
		}

		glNamedBufferStorage(buffer.id, size, data, usageFlags);

		return buffer;
	}


	// query
	Query create_query()
	{
		Query query{};
		glGenQueries(1, &query.id);
		return query;
	}


	// framebuffer
	Framebuffer create_framebuffer()
	{
		Framebuffer result;
		glCreateFramebuffers(1, &result.id);
		return result;
	}


	// fence
	FenceSync create_fence_sync()
	{
		FenceSync sync{};

		// GL_SYNC_GPU_COMMANDS_COMPLETE is the only available option for now
		// flags must be zero for now
		sync.id = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		return sync;
	}


	// buffer range
	BufferRange create_buffer_range(const Buffer& buffer, GLintptr offset, GLsizei size)
	{
		BufferRange range{};
		range.bufferId = buffer.id;
		range.offset   = offset;
		range.size     = size;
		return range;
	}
}
