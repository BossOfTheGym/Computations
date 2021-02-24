#pragma once

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <map>
#include <unordered_map>

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

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace app
{
	// main application class, holds everything inside
	// TODO : everything that is missing
	// TODO : print to console OopenGL context information
	// TODO : resources
	// TODO : text rendering
	// TODO : resource cache
	// TODO : system registry
	// TODO : assets metadata format

	namespace fs = std::filesystem;

	class App
	{
	public:
		static constexpr int HEIGHT = 512;
		static constexpr int WIDTH = 2 * HEIGHT;

		static constexpr i32 TEST_TEXTURE_WIDTH  = 128;
		static constexpr i32 TEST_TEXTURE_HEIGHT = 128;
		static constexpr i32 TEST_TEXTURE_PERIOD = 16;

		static constexpr i32 WORK_X = 16;
		static constexpr i32 WORK_Y = 16;

		static constexpr i32 WX = 511;
		static constexpr i32 WY = 511;
		static constexpr i32 STEPS = 4;
		static constexpr i32 ITERS = 1;

		static constexpr i32 PATCH_X = 32;
		static constexpr i32 PATCH_Y = 32;

		static inline std::string NAME = "computations";

		static inline std::string SHADER_FOLDER = "shaders/";

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
				auto domain = m_redBlackSystem->createAlignedDomain(-1.0, +1.0, -1.0, +1.0, WX, WY);
				auto data = m_redBlackSystem->createAlignedData(domain, m_boundary, m_f);
			
				m_handles[1] = m_redBlackSystem->createSmart(data, STEPS * ITERS);
				m_handles[2] = m_redBlackTiledSystem->createSmart(data, ITERS);
			}

			// *TEST*
			m_proj  = glm::perspective(glm::radians(60.0f), static_cast<f32>(WIDTH / 2) / HEIGHT, 0.1f, 100.0f);
			m_view  = glm::lookAt(glm::vec3(3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			m_model = glm::mat4(1.0f);
			m_model = glm::scale(m_model, glm::vec3(glm::vec2(2.0f), 1.0f));
			m_model = glm::translate(m_model, glm::vec3(-0.5f, -0.5f, 0.0));

			i32 patchVertices = 6 * (PATCH_X - 1) * (PATCH_Y - 1);
			res::Buffer verticesBuffer;
			res::Buffer indicesBuffer;
			res::Buffer uniformBuffer;
			res::VertexArray patch;
			{
				f32 dx = 1.0 / (PATCH_X - 1);
				f32 dy = 1.0 / (PATCH_Y - 1);

				std::vector<glm::vec2> vertices(PATCH_X * PATCH_Y);

				auto vptr = vertices.begin();
				for (i32 j = 0; j < PATCH_Y; j++)
				{
					f32 y = j * dy;
					for (i32 i = 0; i < PATCH_X; i++)
					{
						f32 x = i * dx;

						*vptr++ = glm::vec2(x, y);
					}
				}

				std::vector<u16> indices(6 * (PATCH_X - 1) * (PATCH_Y - 1));

				auto iptr = indices.begin();
				for (i32 j = 0; j < PATCH_Y - 1; j++)
				{
					for (i32 i = 0; i < PATCH_X - 1; i++)
					{
						*iptr++ = j * PATCH_X + i + PATCH_X + 1;
						*iptr++ = j * PATCH_X + i + PATCH_X;
						*iptr++ = j * PATCH_X + i;

						*iptr++ = j * PATCH_X + i + PATCH_X + 1;
						*iptr++ = j * PATCH_X + i;
						*iptr++ = j * PATCH_X + i + 1;
					}
				}

				bool anyFailed = false;
				if (!try_create_storage_buffer(verticesBuffer, vertices.size() * sizeof(glm::vec2), 0, vertices.data()))
				{
					anyFailed = true;

					std::cerr << "Failed to create vertices buffer for patch." << std::endl;
				}
				if (!try_create_storage_buffer(indicesBuffer, indices.size() * sizeof(i16), 0, indices.data()))
				{
					anyFailed = true;

					std::cerr << "Failed to create indices buffer for patch." << std::endl;
				}
				if (!try_create_vertex_array(patch))
				{
					anyFailed = true;

					std::cerr << "Failed to create patch vertex array." << std::endl;
				}
				if (!anyFailed)
				{
					glBindVertexArray(patch.id);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBuffer.id);

					glBindVertexBuffer(0, verticesBuffer.id, 0, sizeof(glm::vec2));
					glEnableVertexAttribArray(0);
					glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
					glVertexAttribBinding(0, 0);
				}

				if (!try_create_storage_buffer(uniformBuffer, 4 * sizeof(glm::mat4), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT))
				{
					std::cerr << "Failed to create uniform buffer" << std::endl;
				}
				else
				{
					auto buffer = (glm::mat4*)glMapNamedBuffer(uniformBuffer.id, GL_WRITE_ONLY);
					buffer[0] = m_proj * m_view * m_model;
					buffer[1] = m_proj;
					buffer[2] = m_view;
					buffer[3] = m_model;
					glUnmapNamedBuffer(uniformBuffer.id);
				}
			}

			// mainloop
			entt::scoped_connection connection{m_mainWindow->keyPressSink().connect<&App::updateSystemsCallback>(this)};

			glEnable(GL_DEPTH_TEST);

			m_mainWindow->show();
			while(!m_mainWindow->shouldClose())
			{
				glfw::poll_events();


				if (!m_paused)
				{
					updateSystems();
				}

				// rendering
				glClearColor(0.1, 0.1, 0.1, 1.0);
				glClearDepth(1.0);
				glClearStencil(0);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				// show program
				glUseProgram(m_shaderPrograms["plot"].id);

				glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer.id);

				glBindVertexArray(patch.id);

				// *TEST*
				glViewport(0, 0, WIDTH / 2, HEIGHT);
				glBindTextureUnit(0, m_handles[1].textureId());
				glDrawElements(GL_TRIANGLES, patchVertices, GL_UNSIGNED_SHORT, nullptr);

				// *TEST*
				glViewport(WIDTH / 2, 0, WIDTH / 2, HEIGHT);
				glBindTextureUnit(0, m_handles[2].textureId());
				glDrawElements(GL_TRIANGLES, patchVertices, GL_UNSIGNED_SHORT, nullptr);

				// swap
				m_mainWindow->swapBuffers();
			}
		}

	private:
		void updateSystemsCallback(int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_U && action == GLFW_PRESS)
			{
				updateSystems();
			}
			if (key == GLFW_KEY_P && action == GLFW_PRESS)
			{
				m_paused = !m_paused;

				std::cout << "paused: " << m_paused << "\n";
			}
			if (key == GLFW_KEY_Q && action == GLFW_PRESS)
			{
				m_mainWindow->shouldClose(true);
			}
		}

		void updateSystems()
		{
			m_redBlackSystem->update();
			m_redBlackTiledSystem->update();

			std::cout << "red-black: " << m_redBlackSystem->timeElapsedMean() / 1000000.0 << "ms \n";
			std::cout << "red-black tiled: " << m_redBlackTiledSystem->timeElapsedMean() / 1000000.0 << "ms \n";
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
			
			m_boundary = [] (f32 x, f32 y) -> f32
			{
				return std::exp(-x * x - y * y);
			};

			m_f = [] (f32 x, f32 y) -> f32
			{
				f32 xxpyy = x * x + y * y;

				return 4.0 * (xxpyy - 1) * std::exp(-xxpyy);
			};

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
			
			std::cout << "*** SHADERS ***" << std::endl;
			for (auto& dirEntry : fs::recursive_directory_iterator(SHADER_FOLDER))
			{
				auto shaderPath = dirEntry.path();
				auto shaderPathStr = shaderPath.string();
				auto shaderName = shaderPathStr.substr(SHADER_FOLDER.size());

				res::Shader shader{};
				GLenum type = res::shader_type_from_extension(shaderPath);
				if (type != -1 && try_create_shader_from_file(shader, type, shaderPath))
				{
					std::cout << "Loaded shader " << std::quoted(shaderPathStr) << std::endl;
					
					m_shaders[shaderName] = std::move(shader);
				}
				else
				{
					std::cerr << "Failed to load " << std::quoted(shaderPathStr) << std::endl;
				}
			}
			std::cout << std::endl;

			// shader programs		
			std::cout << "*** PROGRAMS ***" << std::endl;

			std::vector<std::vector<const char*>> programData
			{
				{"quad", "quad.vert", "quad.frag"},
				{"plot", "plot.vert", "plot.frag"},
				{"test_compute", "test_compute.comp"}, 
				{"jacoby", "jacoby.comp"}, 
				{"red_black", "red_black.comp"}, 
				{"red_black_tiled0", "red_black_tiled0.comp"}, 
				{"red_black_tiled1", "red_black_tiled1.comp"},
				{"red_black_tiled2", "red_black_tiled2.comp"}
			};

			std::vector<res::Shader*> requiredShaders;
			for (auto& data : programData)
			{
				if (data.empty())
				{
					std::cerr << "Invalid data found while attempting to create shader program." << std::endl;

					continue;
				}

				auto programName = data[0];

				requiredShaders.clear();
				for (i32 i = 1; i < data.size(); i++)
				{
					if (auto it = m_shaders.find(data[i]); it != m_shaders.end())
					{
						requiredShaders.push_back(&it->second);
					}
					else
					{
						std::cerr << "Failed to find shader " << std::quoted(data[i]) << std::endl;

						break;
					}
				}

				if (requiredShaders.size() + 1 != data.size())
				{
					std::cerr << "Failed to create program " << std::quoted(programName) << std::endl;

					continue;
				}

				res::ShaderProgram program;
				if (try_create_shader_program(program, requiredShaders.data(), requiredShaders.size()))
				{
					std::cout << std::quoted(programName) << " program created." << std::endl;

					m_shaderPrograms[programName] = std::move(program);
				}
				else
				{
					std::cerr << "Failed to create " << std::quoted(programName) << " shader program." << std::endl;
				}
			}

			std::cout << std::endl;

			// test texture
			std::cout << "*** TEXTURES ***" << std::endl;

			if (!try_create_test_texture(m_texture, TEST_TEXTURE_WIDTH, TEST_TEXTURE_HEIGHT, TEST_TEXTURE_PERIOD))
			{
				std::cerr << "Failed to create test texture." << std::endl;

				return false;
			}
			std::cout << "\"texture\" created." << std::endl;

			std::cout << std::endl;

			// vertex array
			std::cout << "*** VERTEX ARRAYS ***" << std::endl;

			if (!try_create_vertex_array(m_array))
			{
				std::cerr << "Failed to create vertex array." << std::endl;
			}
			std::cout << "\"array\" vertex array created." << std::endl;

			std::cout << std::endl;

			return true;
		}

		void deinitGraphicalResources()
		{
			m_array.reset();

			m_texture.reset();

			m_shaders.clear();
			m_shaderPrograms.clear();
		}

		bool initSystems()
		{
			if (auto it = m_shaderPrograms.find("jacoby"); it != m_shaderPrograms.end())
			{
				m_jacobySystem.reset(new dir2d::JacobyMethod(*this, 16, 16, std::move(it->second)));
				if (m_jacobySystem == nullptr || !m_jacobySystem->programValid())
				{
					std::cerr << "Failed to initialize jacoby system" << std::endl;

					return false;
				}	

				m_shaderPrograms.erase(it);
			}
			else
			{
				std::cerr << std::quoted("jacoby") << " shader program wasn't created." << std::endl;

				return false;
			}

			if (auto it = m_shaderPrograms.find("red_black"); it != m_shaderPrograms.end())
			{
				m_redBlackSystem.reset(new dir2d::RedBlackMethod(*this, WORK_X, WORK_Y, std::move(it->second)));
				if (m_redBlackSystem == nullptr || !m_redBlackSystem->programValid())
				{
					std::cerr << "Failed to initialize red-black system" << std::endl;

					return false;
				}	

				m_shaderPrograms.erase(it);
			}
			else
			{
				std::cerr << std::quoted("red_black") << " shader program wasn't created." << std::endl;

				return false;
			}
			 
			if (auto it = m_shaderPrograms.find("red_black_tiled1"); it != m_shaderPrograms.end())
			{
				m_redBlackTiledSystem.reset(new dir2d::RedBlackTiledMethod(*this, WORK_X, WORK_Y, std::move(it->second)));
				if (m_redBlackTiledSystem == nullptr || !m_redBlackTiledSystem->programValid())
				{
					std::cerr << "Failed to initialize red-black tiled system" << std::endl;

					return false;
				}	

				m_shaderPrograms.erase(it);
			}
			else
			{
				std::cerr << std::quoted("red_black_tiled1") << " shader program wasn't created." << std::endl;

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

			m_redBlackSystem.reset();
			m_jacobySystem.reset();
		}


	private:
		bool m_initialized{false};

		entt::registry   m_registry;
		entt::dispatcher m_dispatcher;

		// resources 
		std::unordered_map<std::string, res::Shader>        m_shaders;
		std::unordered_map<std::string, res::ShaderProgram> m_shaderPrograms;

		res::Texture m_texture;

		res::VertexArray m_array;

		// CORE SYSTEMS
		std::unique_ptr<win::MainWindow> m_mainWindow;

		// SYSTEMS
		dir2d::Function2D m_boundary;
		dir2d::Function2D m_f;

		std::unique_ptr<dir2d::JacobyMethod> m_jacobySystem;
		std::unique_ptr<dir2d::RedBlackMethod> m_redBlackSystem;
		std::unique_ptr<dir2d::RedBlackTiledMethod> m_redBlackTiledSystem;

		// state
		dir2d::SmartHandle m_handles[3]{};
		
		glm::mat4 m_proj;
		glm::mat4 m_view;
		glm::mat4 m_model;

		bool m_paused{true};
	};
}
