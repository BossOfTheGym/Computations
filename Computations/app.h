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
		App();

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

	private:
		void initWindow();
		void initGraphicalResources();
		void initSystems();

		void deinitWindow();
		void deinitGraphicalResources();
		void deinitSystems();

	private:
		void loadShader(const fs::path& path);

		void createProgram(const std::vector<const char*>& data);

	public: 
		res::Id getProgramId(const std::string& program);

	private:
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
