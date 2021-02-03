#include <iostream>
#include <cstdint>

#include "main-window.h"
#include "gl-header.h"
#include "graphics-res.h"
#include "graphics-res-util.h"
#include "dirichlet-2d.h"


int main()
{
	// lib init
	if (!glfw::initialize())
	{
		std::cerr << "Failed to initialize library." << std::endl;
		
		return 1;
	}


	// window
	const int HEIGHT = 512;
	const int WIDTH = 2 * HEIGHT;

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
	// shaders
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

	// shader programs
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

	// test texture
	res::Texture texture{};
	if (!try_create_test_texture(texture, WIDTH, HEIGHT))
	{
		std::cerr << "Failed to create test texture." << std::endl;
	}
	std::cout << "\"texture\" created." << std::endl;

	// vertex array
	res::VertexArray array;
	if (!try_create_vertex_array(array))
	{
		std::cerr << "Failed to create vertex array." << std::endl;
	}
	std::cout << "\"array\" vertex array created." << std::endl;


	// dirichlet solvers
	dir2d::Jacoby jacobyMethod; 
	jacobyMethod.setupProgram(std::move(jacobyProgram));
	if (!jacobyMethod.programValid())
	{
		std::cerr << "Jacoby program is invalid." << std::endl;

		return 1;
	}
	std::cout << "Jacoby program is set up." << std::endl;

	Handle jacobyHandle{null};
	{
		auto domain = dir2d::Jacoby::create_domainAabb2D(-1.1, +1.1, -1.1, +1.1, 511, 511);
		auto data   = dir2d::Jacoby::create_dataAabb2D(domain);
		jacobyHandle = jacobyMethod.create(domain, data, 4);
	}

	dir2d::RedBlack redBlackMethod;
	redBlackMethod.setupProgram(std::move(redBlackProgram));
	if (!redBlackMethod.programValid())
	{
		std::cerr << "Red-black program is invalid." << std::endl;

		return 1;
	}
	std::cout << "Red-black program is set up." << std::endl;

	Handle redBlackHandle{null};
	{
		auto domain = dir2d::RedBlack::create_domainAabb2D(-1.1, +1.1, -1.1, +1.1, 511, 511);
		auto data   = dir2d::RedBlack::create_dataAabb2D(domain);
		redBlackHandle = redBlackMethod.create(domain, data, 2);
	}

	// mainloop
	window.show();
	while(!window.shouldClose())
	{
		glfw::poll_events();

		// compute
		jacobyMethod.setup();
		jacobyMethod.update();

		redBlackMethod.setup();
		redBlackMethod.update();

		// rendering
		glClearColor(1.0, 0.5, 0.2, 1.0);
		glClearDepth(1.0);
		glClearStencil(0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// show program
		glUseProgram(showProgram.id);

		glBindVertexArray(array.id);

		// TODO : move to sampler, use static polymorphism
		glViewport(0, 0, WIDTH / 2, HEIGHT);
		{
			auto& data = jacobyMethod.get(jacobyHandle);
			glBindTextureUnit(0, data.iteration[data.prev].id); 
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// TODO : move to sampler, use static polymorphism
		glViewport(WIDTH / 2, 0, WIDTH / 2, HEIGHT);
		{
			auto& data = redBlackMethod.get(redBlackHandle);
			glBindTextureUnit(0, data.iteration.id); 
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// swap
		window.swapBuffers();
	}


	// lib deinit
	glfw::terminate();

	return 0;
}