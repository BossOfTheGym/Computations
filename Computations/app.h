#pragma once

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <map>
#include <unordered_map>

#include <core.h>

#include <gl-cxx/gl-res.h>
#include <gl-cxx/gl-res-util.h>
#include <gl-cxx/gl-state-info.h>
#include <gl-cxx/gl-header.h>

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
		void initWindow();
		void deinitWindow();

	private:
		std::unique_ptr<win::MainWindow> m_mainWindow;
		std::unique_ptr<gl::GlStateInfo> m_glStateInfo;
	};
}
