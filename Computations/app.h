#pragma once

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <map>
#include <unordered_map>

#include "core.h"
#include "gl-state-info.h"
#include "graphics-res.h"
#include "graphics-res-util.h"
#include "main-window.h"
#include "dirichlet-2d.h"
#include "dirichlet_system.h"

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
	// TODO : split this shit up
	// TODO : small tiling
	// TODO : app class that accepts options as parameters

	namespace fs = std::filesystem;

	// main application class, holds everything inside
	class App
	{
	public:
		static constexpr int HEIGHT = 720;
		static constexpr int WIDTH = 2 * HEIGHT;

		static constexpr i32 TEST_TEXTURE_WIDTH  = 128;
		static constexpr i32 TEST_TEXTURE_HEIGHT = 128;
		static constexpr i32 TEST_TEXTURE_PERIOD = 16;

		static constexpr i32 WORK_X = 16;
		static constexpr i32 WORK_Y = 16;
		
		static constexpr i32 WX = 63;
		static constexpr i32 WY = 63;
		static constexpr i32 STEPS = 2 * 2;
		static constexpr i32 ITERS = 100;


		static inline std::string NAME = "computations";

		static inline std::string SHADER_FOLDER = "shaders/";

	public:
		App() = default;

		App(const App&) = delete;
		App(App&&) = delete;

		App& operator = (const App&) = delete;
		App& operator = (App&&) = delete;

		~App();


	public:
		void mainloop();

	private:
		void updateSystemsCallback(int key, int scancode, int action, int mods);

		void updateSystems();


	public:
		bool init();

		void deinit();

	private:
		bool initWindow();

		void deinitWindow();

		bool initGraphicalResources();

		void deinitGraphicalResources();

		void loadShader(const fs::path& path);

		void createProgram(const std::vector<const char*>& data);

		bool initSystems();
		void deinitSystems();


	public: // NOTE : must be part of resource management module
		res::Id getProgramId(const std::string& program);

	private:
		bool m_initialized{false};

		entt::registry   m_registry;
		entt::dispatcher m_dispatcher;

		// NOTE : must be part of graphics system of some resource management module
		// resources 
		std::unordered_map<std::string, res::Shader>        m_shaders;
		std::unordered_map<std::string, res::ShaderProgram> m_shaderPrograms;
		res::VertexArray m_dummy;
		res::Texture     m_testTex;

		// CORE SYSTEMS
		std::unique_ptr<win::MainWindow> m_mainWindow;
		std::unique_ptr<res::GlStateInfo> m_glStateInfo;

		// SYSTEMS
		std::unique_ptr<DirichletSystem> m_dirichletSystem;

		// state
		dir2d::SmartHandle m_handles[3]{}; // TODO : remove, it should be component in registry ot something else
		
		bool m_paused{true};
	};
}
