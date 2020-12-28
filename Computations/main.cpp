#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <cmath>
#include <array>
#include <cstdint>

#include "MainWindow.h"
#include "gl-header.h"
#include "GraphicsResources.h"


namespace fs = std::filesystem;

using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using f32 = float;
using f64 = double;


// utility
std::optional<std::string> readFileContents(const fs::path& path)
{
	auto filePath = path.string();

	std::ifstream input(filePath);
	if (!input.is_open())
	{
		std::cerr << "Failed to open file " << std::quoted(filePath) << std::endl;

		return std::nullopt;				
	}

	auto start  = std::istreambuf_iterator<char>(input);
	auto finish = std::istreambuf_iterator<char>();
	std::string contents(start, finish);

	return contents;
}


// shader
std::string getShaderInfoLog(const res::Shader& shader)
{
	GLint length{};
	glGetShaderiv(shader.id, GL_INFO_LOG_LENGTH, &length);

	std::string infoLog; 
	infoLog.resize(length);
	glGetShaderInfoLog(shader.id, length, nullptr, infoLog.data());

	return infoLog;
}

res::Shader createShaderFromSource(GLenum shaderType, const std::string& source)
{
	res::Shader shader;

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
		std::cerr << getShaderInfoLog(shader) << std::endl;

		return res::Shader{};
	}

	return shader;
}


// shader program
std::string getShaderProgramInfoLog(const res::ShaderProgram& program)
{
	GLint length{};
	glGetProgramiv(program.id, GL_INFO_LOG_LENGTH, &length);

	std::string infoLog;
	infoLog.resize(length);
	glGetProgramInfoLog(program.id, length, nullptr, infoLog.data());

	return infoLog;
}

template<class ... Shader>
res::ShaderProgram createShaderProgram(Shader&& ... shader)
{
	res::ShaderProgram shaderProgram{};

	shaderProgram.id = glCreateProgram();
	(glAttachShader(shaderProgram.id, shader.id), ...);
	glLinkProgram(shaderProgram.id);
	
	GLint linkStatus{};
	glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		std::cerr << "Failed to link shader program. Error log: " << std::endl;
		std::cerr << getShaderProgramInfoLog(shaderProgram) << std::endl;

		return res::ShaderProgram{};
	}

	(glDetachShader(shaderProgram.id, shader.id), ...);

	return shaderProgram;
}


// texture
res::Texture createFormatTexture(i32 width, i32 height, GLenum format)
{
	res::Texture texture{};

	glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
	glTextureStorage2D(texture.id, 1, format, width, height);
	glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return texture;
}

res::Texture createRgba32fTexture(i32 width, i32 height)
{
	return createFormatTexture(width, height, GL_RGBA32F);
}

res::Texture createTestTexture(i32 width, i32 height)
{
	res::Texture texture = createRgba32fTexture(width, height);

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


// vertex array
res::VertexArray createVertexArray()
{
	res::VertexArray array{};

	glCreateVertexArrays(1, &array.id);

	return array;
}


// buffer
res::Buffer createBuffer(GLsizeiptr size, GLbitfield usageFlags)
{
	res::Buffer buffer{};

	glCreateBuffers(1, &buffer.id);
	glNamedBufferStorage(buffer.id, size, nullptr, usageFlags);

	return buffer;
}


// Problem description
// div(grad(u)) = f
// u(boundary) = g
// first coord is y(rows), second coord is x(cols) as everything is stored in row-major manner
// TODO : think of additional texture for f-function
// texture is 'padded' with boundary conditions
struct Problem
{
	res::Texture textures[2]{};

	f32 x0{};
	f32 x1{};
	f32 y0{};
	f32 y1{};
	f32 hx{};
	f32 hy{};
	i32 xSplit{};
	i32 ySplit{};
};

Problem createProblem(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
{	
	f32 hx = (x1 - x0) / xSplit;
	f32 hy = (y1 - y0) / ySplit;

	Problem problem;
	problem.textures[0] = createFormatTexture(xSplit + 1, ySplit + 1, GL_R32F);
	problem.textures[1] = createFormatTexture(xSplit + 1, ySplit + 1, GL_R32F);
	problem.x0 = x0;
	problem.x1 = x1;
	problem.y0 = y0;
	problem.y1 = y1;
	problem.hx = hx;
	problem.hy = hy;
	problem.xSplit = xSplit;
	problem.ySplit = ySplit;

	std::vector<f32> data((xSplit + 1) * (ySplit + 1));

	auto ptr = data.begin();
	// y0 boundary
	for (i32 j = 0; j <= xSplit; j++)
	{
		f32 x = x0 + j * hx;

		*ptr++ = std::exp(-x * x - y0 * y0);
	}
	for (i32 i = 1; i < ySplit; i++)
	{
		f32 y = y0 + i * hy;

		// x0 boundary
		*ptr++ = std::exp(-x0 * x0 - y * y);
		for (i32 j = 1; j < xSplit; j++)
		{
			f32 x = x0 + j * hx;

			// condition
			*ptr++ = 4.0 * (x * x + y * y - 1.0) * std::exp(-x * x - y * y);
		}
		// x1 boundary
		*ptr++ = std::exp(-x1 * x1 - y * y);
	}
	// y1 boundary
	for (i32 j = 0; j <= xSplit; j++)
	{
		f32 x = x0 + j * hx;

		*ptr++ = std::exp(-x * x - y1 * y1);
	}

	glTextureSubImage2D(problem.textures[0].id, 0, 0, 0, xSplit + 1, ySplit + 1, GL_RED, GL_FLOAT, data.data());
	glTextureSubImage2D(problem.textures[1].id, 0, 0, 0, xSplit + 1, ySplit + 1, GL_RED, GL_FLOAT, data.data());

	return problem;
}


int main()
{
	// lib init
	if (!glfw::initialize())
	{
		std::cerr << "Failed to initialize library." << std::endl;
		
		return 1;
	}


	// window
	const int WIDTH = 1024;
	const int HEIGHT = 1024;

	glfw::CreationInfo info;
	info.width = WIDTH;
	info.height = HEIGHT;
	info.title = "computations";
	info.intHints.push_back({glfw::Resizable, glfw::False});
	info.intHints.push_back({glfw::ContextVersionMajor, 4});
	info.intHints.push_back({glfw::ContextVersionMinor, 6});
	info.intHints.push_back({glfw::DoubleBuffer, glfw::True});

	MainWindow window(info);
	window.makeContextCurrent();


	// resources
	res::Shader quadVert{};
	if (auto contents = readFileContents("shaders/quad.vert"))
	{
		quadVert = createShaderFromSource(GL_VERTEX_SHADER, *contents);	
	}
	if (quadVert.id == res::null)
	{
		std::cerr << "Failed to load \"quad.vert\" ." << std::endl;

		return 1;
	}
	std::cout << "\"quad.vert\" shader created." << std::endl;

	res::Shader quadFrag{};
	if (auto contents = readFileContents("shaders/quad.frag"))
	{
		quadFrag = createShaderFromSource(GL_FRAGMENT_SHADER, *contents);	
	}
	if (quadFrag.id == res::null)
	{
		std::cerr << "Failed to load \"quad.frag\" ." << std::endl;

		return 1;
	}
	std::cout << "\"quad.frag\" shader created." << std::endl;

	res::Shader testCompute{};
	if (auto contents = readFileContents("shaders/test_compute.comp"))
	{
		testCompute = createShaderFromSource(GL_COMPUTE_SHADER, *contents);	
	}
	if (testCompute.id == res::null)
	{
		std::cerr << "Failed to load \"test_compute.comp\" ." << std::endl;

		return 1;
	}
	std::cout << "\"test_compute.comp\" shader created." << std::endl;

	res::Shader testJacoby{};
	if (auto contents = readFileContents("shaders/test_jacoby.comp"))
	{
		testJacoby = createShaderFromSource(GL_COMPUTE_SHADER, *contents);
	}
	if (testJacoby.id == res::null)
	{	
		std::cerr << "Failed to load \"test_jacoby.comp\" ." << std::endl;

		return 1;
	}
	std::cout << "\"test_jacoby.comp\" shader created." << std::endl;

	res::ShaderProgram showProgram = createShaderProgram(quadVert, quadFrag);
	if (showProgram.id == res::null)
	{
		std::cerr << "Failed to create shader program." << std::endl;

		return 1;
	}
	std::cout << "\"showProgram\" program created." << std::endl;

	res::ShaderProgram computeProgram = createShaderProgram(testCompute);
	if (computeProgram.id == res::null)
	{
		std::cerr << "Failed to create \"computeProgram\" program." << std::endl;

		return 1;
	}
	std::cout << "\"computeProgram\" program created." << std::endl;

	res::ShaderProgram jacobyProgram = createShaderProgram(testJacoby);
	if (jacobyProgram.id == res::null)
	{
		std::cerr << "Failed to create \"jacobyProgram\" program." << std::endl;

		return 1;
	}
	std::cout << "\"jacobyProgram\" program created." << std::endl;

	GLint prevLoc = glGetUniformLocation(jacobyProgram.id, "prev");
	GLint nextLoc = glGetUniformLocation(jacobyProgram.id, "next");
	GLint x0Loc = glGetUniformLocation(jacobyProgram.id, "x0");
	GLint y0Loc = glGetUniformLocation(jacobyProgram.id, "y0");
	GLint hxLoc = glGetUniformLocation(jacobyProgram.id, "hx");
	GLint hyLoc = glGetUniformLocation(jacobyProgram.id, "hy");

	res::Texture texture = createTestTexture(WIDTH, HEIGHT);
	std::cout << "\"texture\" created." << std::endl;

	res::VertexArray array = createVertexArray();
	std::cout << "\"array\" vertex array created." << std::endl;

	// problem
	Problem problem = createProblem(-1.2, +1.2, -1.2, +1.2, 256 - 1, 256 - 1); // 256 x 256 vars
	if (problem.textures[0].id == res::null || problem.textures[1].id == res::null)
	{
		std::cerr << "Failed to create \"problem\"" << std::endl;

		return 1;
	}
	std::cout << "\"problem\" created." << std::endl;

	// mainloop
	i32 prev = 0;
	i32 curr = 1;

	window.show();
	while(!window.shouldClose())
	{
		glfw::poll_events();

		glClearColor(1.0, 0.5, 0.2, 1.0);
		glClearDepth(1.0);
		glClearStencil(0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// test compute program
		//glUseProgram(computeProgram.id);
		//glBindImageTexture(0, texture.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		//
		//glDispatchCompute(32, 32, 1);

		// jacoby compute program
		glUseProgram(jacobyProgram.id);
		glBindImageTexture(0, problem.textures[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
		glBindImageTexture(1, problem.textures[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

		glUniform1i(prevLoc, prev);
		glUniform1i(nextLoc, curr);
		glUniform1f(x0Loc, problem.x0);
		glUniform1f(y0Loc, problem.y0);
		glUniform1f(hxLoc, problem.hx);
		glUniform1f(hyLoc, problem.hy);
		glDispatchCompute(16, 16, 1);

		std::swap(prev, curr);

		glUniform1i(prevLoc, prev);
		glUniform1i(nextLoc, curr);
		glUniform1f(x0Loc, problem.x0);
		glUniform1f(y0Loc, problem.y0);
		glUniform1f(hxLoc, problem.hx);
		glUniform1f(hyLoc, problem.hy);
		glDispatchCompute(16, 16, 1);

		std::swap(prev, curr);

		// show program
		glUseProgram(showProgram.id); 
		glBindTextureUnit(0, problem.textures[curr].id); 

		glBindVertexArray(array.id);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// swap
		window.swapBuffers();
	}


	// lib deinit
	glfw::terminate();

	return 0;
}