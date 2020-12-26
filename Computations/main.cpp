#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <cmath>

#include "MainWindow.h"
#include "gl-header.h"
#include "GraphicsResources.h"


namespace fs = std::filesystem;


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
res::Texture createRgba32fTexture(int width, int height)
{
	res::Texture texture{};

	glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
	glTextureStorage2D(texture.id, 1, GL_RGBA32F, width, height);
	glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return texture;
}

res::Texture createTestTexture(int width, int height)
{
	res::Texture texture = createRgba32fTexture(width, height);

	std::vector<float> data(width * height * 4);

	const float PI = 3.14159265359;
	const float K = 2 * PI / 64;

	auto ptr = data.begin();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float c = std::abs(std::sin(K * j) + std::sin(K * i));

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
res::Buffer createBuffer()
{
	// TODO

	return res::Buffer{};
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
		std::cerr << "Failed to create compute shader program." << std::endl;

		return 1;
	}
	std::cout << "\"computeProgram\" program created." << std::endl;

	res::Texture texture = createTestTexture(WIDTH, HEIGHT);
	std::cout << "\"texture\" created." << std::endl;

	res::VertexArray array = createVertexArray();
	std::cout << "\"array\" vertex array created." << std::endl;


	// mainloop
	window.show();
	while(!window.shouldClose())
	{
		glfw::poll_events();

		glClearColor(1.0, 0.5, 0.2, 1.0);
		glClearDepth(1.0);
		glClearStencil(0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// compute program
		glUseProgram(computeProgram.id);
		glBindImageTexture(0, texture.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		glDispatchCompute(32, 32, 1);

		// show program
		glUseProgram(showProgram.id);
		glBindTextureUnit(0, texture.id); 

		glBindVertexArray(array.id);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		window.swapBuffers();
	}


	// lib deinit
	glfw::terminate();

	return 0;
}