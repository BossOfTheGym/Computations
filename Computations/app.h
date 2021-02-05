#pragma once

#include <iostream>

#include "core.h"
#include "graphics-res.h"
#include "graphics-res-util.h"
#include "main-window.h"
#include "dirichlet-2d.h"

#include <entt/entity/storage.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/delegate.hpp>
#include <entt/signal/dispatcher.hpp>

namespace app
{
	// main application class, holds everything inside
	// TODO : everything that is missing
	class App
	{
	public:
		static constexpr int HEIGHT = 512;
		static constexpr int WIDTH = 2 * HEIGHT;

		static inline std::string NAME = "computations";


	public:
		App() = default;

		App(const App&) = delete;
		App(App&&) = delete;

		App& operator = (const App&) = delete;
		App& operator = (App&&) = delete;

		~App()
		{
			if (m_initialized)
			{
				deinit();
			}
		}


	public:
		void mainloop()
		{
			// *TEST*
			Handle jacobyHandle{null};
			{
				auto data = dir2d::JacobyMethod::create_dataAabb2D(-1.2, +1.2, -1.2, +1.2, 511, 511);

				jacobyHandle = m_jacobySystem->create(data, 16);
			}

			// *TEST*
			Handle redBlackHandle{null};
			{
				auto data = dir2d::RedBlackMethod::create_dataAabb2D(-1.2, +1.2, -1.2, +1.2, 511, 511);

				redBlackHandle = m_redBlackSystem->create(data, 2);
			}

			// mainloop
			m_mainWindow->show();
			while(!m_mainWindow->shouldClose())
			{
				glfw::poll_events();

				// system update
				m_jacobySystem->setup();
				m_jacobySystem->update();

				m_redBlackSystem->setup();
				m_redBlackSystem->update();

				// rendering
				glClearColor(1.0, 0.5, 0.2, 1.0);
				glClearDepth(1.0);
				glClearStencil(0);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				// show program
				glUseProgram(m_showProgram.id);
				glBindVertexArray(m_array.id);

				// *TEST*
				glViewport(0, 0, WIDTH / 2, HEIGHT);
				{
					auto& data = m_jacobySystem->get(jacobyHandle);

					glBindTextureUnit(0, data.iteration[data.prev].id);
				}
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				// *TEST*
				glViewport(WIDTH / 2, 0, WIDTH / 2, HEIGHT);
				{
					auto& data = m_redBlackSystem->get(redBlackHandle);

					glBindTextureUnit(0, data.iteration.id);
				}
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				// swap
				m_mainWindow->swapBuffers();
			}
		}


	public:
		bool init()
		{
			m_initialized = initWindow() && initGraphicalResources() && initSystems();
			
			return m_initialized;
		}

		void deinit()
		{
			if (!m_initialized)
			{
				std::cerr << "Warning! Attempt to deinitialize uninitiaized app." << std::endl;

				return;
			}

			deinitSystems();
			deinitGraphicalResources();
			deinitWindow();

			m_initialized = false;
		}

	private:
		bool initWindow()
		{
			if (!glfw::initialize())
			{
				std::cerr << "Failed to initialize library." << std::endl;

				return false;
			}

			glfw::CreationInfo info;
			info.width  = WIDTH;
			info.height = HEIGHT;
			info.title = NAME;
			info.intHints.push_back({glfw::Hint::Resizable, (int)glfw::Value::False});
			info.intHints.push_back({glfw::Hint::ContextVersionMajor, 4});
			info.intHints.push_back({glfw::Hint::ContextVersionMinor, 5});
			info.intHints.push_back({glfw::Hint::DoubleBuffer, (int)glfw::Value::True});

			m_mainWindow.reset(new win::MainWindow(info));
			if (m_mainWindow == nullptr || !m_mainWindow->valid())
			{
				std::cerr << "Failed to create window" << std::endl;

				return false;
			}
			return true;
		}

		void deinitWindow()
		{
			m_mainWindow.reset();

			glfw::terminate();
		}

		bool initGraphicalResources()
		{
			m_mainWindow->makeContextCurrent();

			// TODO : create a list 
			// resources
			// shaders
			if (!try_create_shader_from_file(m_quadVert, GL_VERTEX_SHADER, "shaders/quad.vert"))
			{
				std::cerr << "Failed to load \"quad.vert\" ." << std::endl;

				return false;
			}
			std::cout << "\"quad.vert\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_quadFrag, GL_FRAGMENT_SHADER, "shaders/quad.frag"))
			{
				std::cerr << "Failed to load \"quad.frag\" ." << std::endl;

				return false;
			}
			std::cout << "\"quad.frag\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_testCompute, GL_COMPUTE_SHADER, "shaders/test_compute.comp"))
			{
				std::cerr << "Failed to load \"test_compute.comp\" ." << std::endl;

				return false;
			}
			std::cout << "\"test_compute.comp\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_testJacoby, GL_COMPUTE_SHADER, "shaders/test_jacoby.comp"))
			{	
				std::cerr << "Failed to load \"test_jacoby.comp\" ." << std::endl;

				return 1;
			}
			std::cout << "\"test_jacoby.comp\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_testRedBlack, GL_COMPUTE_SHADER, "shaders/test_red_black.comp"))
			{	
				std::cerr << "Failed to load \"test_red_black.comp\" ." << std::endl;

				return false;
			}
			std::cout << "\"test_red_black.comp\" shader created." << std::endl;

			// shader programs
			if (!try_create_shader_program(m_showProgram, m_quadVert, m_quadFrag))
			{
				std::cerr << "Failed to create shader program." << std::endl;

				return false;
			}
			std::cout << "\"showProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_computeProgram, m_testCompute))
			{
				std::cerr << "Failed to create \"computeProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"computeProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_jacobyProgram, m_testJacoby))
			{
				std::cerr << "Failed to create \"jacobyProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"jacobyProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_redBlackProgram, m_testRedBlack))
			{
				std::cerr << "Failed to create \"redBlackProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"redBlackProgram\" program created." << std::endl;

			// test texture
			if (!try_create_test_texture(m_texture, WIDTH, HEIGHT))
			{
				std::cerr << "Failed to create test texture." << std::endl;

				return false;
			}
			std::cout << "\"texture\" created." << std::endl;

			// vertex array
			if (!try_create_vertex_array(m_array))
			{
				std::cerr << "Failed to create vertex array." << std::endl;
			}
			std::cout << "\"array\" vertex array created." << std::endl;

			return true;
		}

		void deinitGraphicalResources()
		{
			m_array.reset();

			m_texture.reset();

			m_redBlackProgram.reset();
			m_jacobyProgram.reset();
			m_showProgram.reset();
			m_computeProgram.reset();

			m_testRedBlack.reset();
			m_testJacoby.reset();
			m_testCompute.reset();
			m_quadFrag.reset();
			m_quadVert.reset();
		}

		bool initSystems()
		{
			m_jacobySystem.reset(new dir2d::JacobyMethod(*this, std::move(m_jacobyProgram)));
			if (m_jacobySystem == nullptr || !m_jacobySystem->programValid())
			{
				std::cerr << "Failed to initialize Jacoby system" << std::endl;

				return false;
			}

			m_redBlackSystem.reset(new dir2d::RedBlackMethod(*this, std::move(m_redBlackProgram)));
			if (m_redBlackSystem == nullptr || !m_redBlackSystem->programValid())
			{
				std::cerr << "Failed to initialize Red-black system" << std::endl;

				return false;
			}

			return true;
		}

		void deinitSystems()
		{
			m_jacobySystem.reset();
		}


	private:
		bool m_initialized{false};

		entt::registry   m_registry;
		entt::dispatcher m_dispatcher;

		// TODO : it must be stored in a resource cache
		// resources
		res::Shader m_quadVert;
		res::Shader m_quadFrag;
		res::Shader m_testCompute;
		res::Shader m_testJacoby;
		res::Shader m_testRedBlack;

		res::ShaderProgram m_computeProgram;
		res::ShaderProgram m_showProgram;
		res::ShaderProgram m_jacobyProgram;
		res::ShaderProgram m_redBlackProgram;

		res::Texture m_texture;

		res::VertexArray m_array;

		// CORE SYSTEMS
		// TODO : system registry
		std::unique_ptr<win::MainWindow> m_mainWindow;

		// TODO : system registry
		// SYSTEMS
		std::unique_ptr<dir2d::JacobyMethod> m_jacobySystem;
		std::unique_ptr<dir2d::RedBlackMethod> m_redBlackSystem;
	};
}