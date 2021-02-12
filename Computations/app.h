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
	// TODO : print to console OopenGL context information
	class App
	{
	public:
		static constexpr int HEIGHT = 800;
		static constexpr int WIDTH = 2 * HEIGHT;
		static constexpr i32 WX = 255;
		static constexpr i32 WY = 255;
		static constexpr i32 STEPS = 4;

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
			{
				auto data = dir2d::RedBlackMethod::create_dataAabb2D(-1.2, +1.2, -1.2, +1.2, WX, WY);

				m_handles[1] = m_redBlackSystem->createSmart(data, STEPS);
			}
			{
				auto data = dir2d::RedBlackTiledMethod::create_dataAabb2D(-1.2, +1.2, -1.2, +1.2, WX, WY);

				m_handles[2] = m_redBlackTiledSystem->createSmart(data, 1);
			}

			// mainloop
			entt::scoped_connection connection{m_mainWindow->keyPressSink().connect<&App::updateSystems>(this)};

			m_mainWindow->show();
			while(!m_mainWindow->shouldClose())
			{
				glfw::poll_events();

				// rendering
				glClearColor(1.0, 0.5, 0.2, 1.0);
				glClearDepth(1.0);
				glClearStencil(0);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				// show program
				glUseProgram(m_showProgram.id);
				glBindVertexArray(m_array.id);

				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				// *TEST*
				glViewport(0, 0, WIDTH / 2, HEIGHT);
				glBindTextureUnit(0, m_handles[1].textureId());
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				// *TEST*
				glViewport(WIDTH / 2, 0, WIDTH / 2, HEIGHT);
				glBindTextureUnit(0, m_handles[2].textureId());
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				// swap
				m_mainWindow->swapBuffers();
			}
		}

	private:
		void updateSystems(int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_U && action == GLFW_PRESS)
			{
				m_jacobySystem->update();
				m_redBlackSystem->update();
				m_redBlackTiledSystem->update();
			}
			if (key == GLFW_KEY_R && action == GLFW_PRESS)
			{
				reloadRedBlackTiled();
			}
		}

		void reloadRedBlackTiled()
		{
			m_redBlackTiled.reset();
			if (!try_create_shader_from_file(m_redBlackTiled, GL_COMPUTE_SHADER, "shaders/red_black_tiled.comp"))
			{	
				std::cerr << "Failed to reload \"shaders/red_black_tiled.comp\" ." << std::endl;
			}
			std::cout << "\"shaders/red_black_tiled.comp\" shader recreated." << std::endl;

			m_redBlackTiledProgram.reset();
			if (!try_create_shader_program(m_redBlackTiledProgram, m_redBlackTiled))
			{
				std::cerr << "Failed to recreate \"redBlackTiledProgram\" program." << std::endl;
			}
			std::cout << "\"redBlackTiledProgram\" program recreated." << std::endl;

			m_redBlackTiledSystem->setupProgram(std::move(m_redBlackTiledProgram));

			{
				auto data = dir2d::RedBlackMethod::create_dataAabb2D(-1.2, +1.2, -1.2, +1.2, WX, WY);

				m_handles[1] = m_redBlackSystem->createSmart(data, STEPS);
			}
			{
				auto data = dir2d::RedBlackTiledMethod::create_dataAabb2D(-1.2, +1.2, -1.2, +1.2, WX, WY);

				m_handles[2] = m_redBlackTiledSystem->createSmart(data, 1);
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

			if (!try_create_shader_from_file(m_jacoby, GL_COMPUTE_SHADER, "shaders/jacoby.comp"))
			{	
				std::cerr << "Failed to load \"jacoby.comp\" ." << std::endl;

				return 1;
			}
			std::cout << "\"jacoby.comp\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_redBlack, GL_COMPUTE_SHADER, "shaders/red_black.comp"))
			{	
				std::cerr << "Failed to load \"red_black.comp\" ." << std::endl;

				return false;
			}
			std::cout << "\"red_black.comp\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_mirroredRedBlack, GL_COMPUTE_SHADER, "shaders/mirror_red_black.comp"))
			{	
				std::cerr << "Failed to load \"mirror_red_black.comp\" ." << std::endl;

				return false;
			}
			std::cout << "\"mirror_red_black.comp\" shader created." << std::endl;

			if (!try_create_shader_from_file(m_redBlackTiled, GL_COMPUTE_SHADER, "shaders/red_black_tiled.comp"))
			{	
				std::cerr << "Failed to load \"shaders/red_black_tiled.comp\" ." << std::endl;

				return false;
			}
			std::cout << "\"shaders/red_black_tiled.comp\" shader created." << std::endl;

			// shader programs
			if (!try_create_shader_program(m_showProgram, m_quadVert, m_quadFrag))
			{
				std::cerr << "Failed to create shader program." << std::endl;

				return false;
			}
			std::cout << "\"showProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_testComputeProgram, m_testCompute))
			{
				std::cerr << "Failed to create \"computeProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"computeProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_jacobyProgram, m_jacoby))
			{
				std::cerr << "Failed to create \"jacobyProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"jacobyProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_redBlackProgram, m_redBlack))
			{
				std::cerr << "Failed to create \"redBlackProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"redBlackProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_mirroredRedBlackProgram, m_mirroredRedBlack))
			{
				std::cerr << "Failed to create \"mirroredRedBlackProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"mirroredRedBlackProgram\" program created." << std::endl;

			if (!try_create_shader_program(m_redBlackTiledProgram, m_redBlackTiled))
			{
				std::cerr << "Failed to create \"redBlackTiledProgram\" program." << std::endl;

				return false;
			}
			std::cout << "\"redBlackTiledProgram\" program created." << std::endl;

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

			m_redBlackTiledProgram.reset();
			m_mirroredRedBlackProgram.reset();
			m_redBlackProgram.reset();
			m_jacobyProgram.reset();
			m_showProgram.reset();
			m_testComputeProgram.reset();

			m_redBlackTiled.reset();
			m_mirroredRedBlack.reset();
			m_redBlack.reset();
			m_jacoby.reset();
			m_testCompute.reset();
			m_quadFrag.reset();
			m_quadVert.reset();
		}

		bool initSystems()
		{
			m_jacobySystem.reset(new dir2d::JacobyMethod(*this, std::move(m_jacobyProgram)));
			if (m_jacobySystem == nullptr || !m_jacobySystem->programValid())
			{
				std::cerr << "Failed to initialize jacoby system" << std::endl;

				return false;
			}

			m_redBlackSystem.reset(new dir2d::RedBlackMethod(*this, std::move(m_redBlackProgram)));
			if (m_redBlackSystem == nullptr || !m_redBlackSystem->programValid())
			{
				std::cerr << "Failed to initialize red-black system" << std::endl;

				return false;
			}

			m_mirroredRedBlackSystem.reset(new dir2d::MirroredRedBlackMethod(*this, std::move(m_mirroredRedBlackProgram)));
			if (m_mirroredRedBlackSystem == nullptr || !m_mirroredRedBlackSystem->programValid())
			{
				std::cerr << "Failed to initialize mirrored red-black system" << std::endl;

				return false;
			}

			m_redBlackTiledSystem.reset(new dir2d::RedBlackTiledMethod(*this, std::move(m_redBlackTiledProgram)));
			if (m_redBlackTiledSystem == nullptr || !m_redBlackTiledSystem->programValid())
			{
				std::cerr << "Failed to initialize red-black tiled system" << std::endl;

				return false;
			}

			return true;
		}

		void deinitSystems()
		{
			for (auto& handle : m_handles)
			{
				handle.reset();
			}

			m_mirroredRedBlackSystem.reset();
			m_redBlackSystem.reset();
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
		res::Shader m_jacoby;
		res::Shader m_redBlack;
		res::Shader m_mirroredRedBlack;
		res::Shader m_redBlackTiled;

		res::ShaderProgram m_testComputeProgram;
		res::ShaderProgram m_showProgram;
		res::ShaderProgram m_jacobyProgram;
		res::ShaderProgram m_redBlackProgram;
		res::ShaderProgram m_mirroredRedBlackProgram;
		res::ShaderProgram m_redBlackTiledProgram;

		res::Texture m_texture;

		res::VertexArray m_array;

		// CORE SYSTEMS
		// TODO : system registry
		std::unique_ptr<win::MainWindow> m_mainWindow;

		// TODO : system registry
		// SYSTEMS
		std::unique_ptr<dir2d::JacobyMethod> m_jacobySystem;
		std::unique_ptr<dir2d::RedBlackMethod> m_redBlackSystem;
		std::unique_ptr<dir2d::MirroredRedBlackMethod> m_mirroredRedBlackSystem;
		std::unique_ptr<dir2d::RedBlackTiledMethod> m_redBlackTiledSystem;

		dir2d::SmartHandle m_handles[3]{};
	};
}
