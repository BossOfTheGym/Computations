#include <iostream>
#include <cstdint>

#include "main-window.h"
#include "gl-header.h"
#include "graphics-res.h"
#include "graphics-res-util.h"


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
	info.intHints.push_back({glfw::ContextVersionMinor, 5});
	info.intHints.push_back({glfw::DoubleBuffer, glfw::True});

	MainWindow window(info);
	window.makeContextCurrent();


	// resources
	res::Shader quadVert{};
	if (!try_create_shader_from_file(quadVert, GL_VERTEX_SHADER, "shaders/quad.vert"))
	{
		std::cerr << "Failed to load \"quad.vert\" ." << std::endl;

		return 1;
	}
	std::cout << "\"quad.vert\" shader created." << std::endl;

	res::Shader quadFrag{};
	if (!try_create_shader_from_file(quadFrag, GL_FRAGMENT_SHADER, "shaders/quad.frag"))
	{
		std::cerr << "Failed to load \"quad.frag\" ." << std::endl;

		return 1;
	}
	std::cout << "\"quad.frag\" shader created." << std::endl;

	res::Shader testCompute{};
	if (!try_create_shader_from_file(testCompute, GL_COMPUTE_SHADER, "shaders/test_compute.comp"))
	{
		std::cerr << "Failed to load \"test_compute.comp\" ." << std::endl;

		return 1;
	}
	std::cout << "\"test_compute.comp\" shader created." << std::endl;

	res::Shader testJacoby{};
	if (!try_create_shader_from_file(testJacoby, GL_COMPUTE_SHADER, "shaders/test_jacoby.comp"))
	{	
		std::cerr << "Failed to load \"test_jacoby.comp\" ." << std::endl;

		return 1;
	}
	std::cout << "\"test_jacoby.comp\" shader created." << std::endl;

	res::Shader testRedBlack{};
	if (!try_create_shader_from_file(testRedBlack, GL_COMPUTE_SHADER, "shaders/test_red_black.comp"))
	{	
		std::cerr << "Failed to load \"test_red_black.comp\" ." << std::endl;

		return 1;
	}
	std::cout << "\"test_red_black.comp\" shader created." << std::endl;


	res::ShaderProgram showProgram{};
	if (!try_create_shader_program(showProgram, quadVert, quadFrag))
	{
		std::cerr << "Failed to create shader program." << std::endl;

		return 1;
	}
	std::cout << "\"showProgram\" program created." << std::endl;

	res::ShaderProgram computeProgram{};
	if (!try_create_shader_program(computeProgram, testCompute))
	{
		std::cerr << "Failed to create \"computeProgram\" program." << std::endl;

		return 1;
	}
	std::cout << "\"computeProgram\" program created." << std::endl;

	res::ShaderProgram jacobyProgram{};
	if (!try_create_shader_program(jacobyProgram, testJacoby))
	{
		std::cerr << "Failed to create \"jacobyProgram\" program." << std::endl;

		return 1;
	}
	std::cout << "\"jacobyProgram\" program created." << std::endl;

	res::ShaderProgram redBlackProgram{};
	if (!try_create_shader_program(redBlackProgram, testRedBlack))
	{
		std::cerr << "Failed to create \"redBlackProgram\" program." << std::endl;

		return 1;
	}
	std::cout << "\"redBlackProgram\" program created." << std::endl;


	res::Texture texture{};
	if (!try_create_test_texture(texture, WIDTH, HEIGHT))
	{
		std::cerr << "Failed to create test texture." << std::endl;
	}
	std::cout << "\"texture\" created." << std::endl;


	res::VertexArray array;
	if (!try_create_vertex_array(array))
	{
		std::cerr << "Failed to create vertex array." << std::endl;
	}
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

		// show program
		glUseProgram(showProgram.id); 
		glBindTextureUnit(0, texture.id); 

		glBindVertexArray(array.id);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// swap
		window.swapBuffers();
	}


	// lib deinit
	glfw::terminate();

	return 0;
}