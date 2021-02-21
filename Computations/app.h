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
	// TODO : resources
	// TODO : text rendering

	class App
	{
	public:
		static constexpr int WIDTH = 512;
		static constexpr int HEIGHT = 2 * WIDTH;

		static constexpr i32 TEST_TEXTURE_WIDTH  = 128;
		static constexpr i32 TEST_TEXTURE_HEIGHT = 128;
		static constexpr i32 TEST_TEXTURE_PERIOD = 16;

		static constexpr i32 WORK_X = 16;
		static constexpr i32 WORK_Y = 16;

		static constexpr i32 WX = 511;
		static constexpr i32 WY = 511;
		static constexpr i32 STEPS = 8;

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
				auto domain = m_redBlackSystem->createAlignedDomain(-1.2, +1.2, -1.2, +1.2, WX, WY);
				auto data = m_redBlackSystem->createAlignedData(domain, m_boundary, m_f);
			
				m_handles[1] = m_redBlackSystem->createSmart(data, STEPS);
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
				glViewport(0, 0, WIDTH, HEIGHT / 2);
				glBindTextureUnit(0, m_handles[1].textureId());
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				// *TEST*
				glViewport(0, HEIGHT / 2, WIDTH, HEIGHT / 2);
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
				std::cout << "jacoby: " <<  m_jacobySystem->timeElapsed() / 1000000.0 << "ms \n";
				std::cout << "red-black: " << m_redBlackSystem->timeElapsed() / 1000000.0 << "ms \n";
				std::cout << "red-black tiled: " << m_redBlackTiledSystem->timeElapsed() / 1000000.0 << "ms \n";

				m_jacobySystem->update();
				m_redBlackSystem->update();
				m_redBlackTiledSystem->update();
			}
		}


	public:
		bool init()
		{
			if (m_initialized)
			{
				std::cout << "[WARNING] App already initialized." << std::endl;

				return true;
			}

			if (!initWindow())
			{
				std::cerr << "Failed to initialize window." << std::endl;

				return false;
			}

			if (!initGraphicalResources())
			{
				std::cerr << "Failed to initialize graphical resources." << std::endl;

				return false;
			}

			if (!initSystems())
			{
				std::cerr << "Failed to initialize systems." << std::endl;

				return false;
			}
			
			m_initialized = true;

			return true;
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

			if (!try_create_shader_from_file(m_redBlackTiled, GL_COMPUTE_SHADER, "shaders/red_black_tiled0.comp"))
			{	
				std::cerr << "Failed to load \"shaders/red_black_tiled0.comp\" ." << std::endl;

				return false;
			}
			std::cout << "\"shaders/red_black_tiled0.comp\" shader created." << std::endl;

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
			if (!try_create_test_texture(m_texture, TEST_TEXTURE_WIDTH, TEST_TEXTURE_HEIGHT, TEST_TEXTURE_PERIOD))
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
			m_jacobySystem.reset(new dir2d::JacobyMethod(*this, 16, 16, std::move(m_jacobyProgram)));
			if (m_jacobySystem == nullptr || !m_jacobySystem->programValid())
			{
				std::cerr << "Failed to initialize jacoby system" << std::endl;

				return false;
			}

			m_redBlackSystem.reset(new dir2d::RedBlackMethod(*this, WORK_X, WORK_Y, std::move(m_redBlackProgram)));
			if (m_redBlackSystem == nullptr || !m_redBlackSystem->programValid())
			{
				std::cerr << "Failed to initialize red-black system" << std::endl;

				return false;
			}

			m_redBlackTiledSystem.reset(new dir2d::RedBlackTiledMethod(*this, WORK_X, WORK_Y, std::move(m_redBlackTiledProgram)));
			if (m_redBlackTiledSystem == nullptr || !m_redBlackTiledSystem->programValid())
			{
				std::cerr << "Failed to initialize red-black tiled system" << std::endl;

				return false;
			}

			m_boundary = [] (f32 x, f32 y) -> f32
			{
				return std::exp(-x * x - y * y);
			};

			m_f = [] (f32 x, f32 y) -> f32
			{
				f32 xxpyy = x * x + y * y;

				return 4.0 * (xxpyy - 1) * std::exp(-xxpyy);
			};

			return true;
		}

		void deinitSystems()
		{
			for (auto& handle : m_handles)
			{
				handle.reset();
			}

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
		dir2d::Function2D m_boundary;
		dir2d::Function2D m_f;

		std::unique_ptr<dir2d::JacobyMethod> m_jacobySystem;
		std::unique_ptr<dir2d::RedBlackMethod> m_redBlackSystem;
		std::unique_ptr<dir2d::RedBlackTiledMethod> m_redBlackTiledSystem;

		dir2d::SmartHandle m_handles[3]{};
	};
}
