#include "app.h"

#include <exception>

namespace app
{
	/*constexpr int HEIGHT = 800;
	constexpr int WIDTH = 2 * HEIGHT;

	constexpr i32 TEST_TEXTURE_WIDTH  = 128;
	constexpr i32 TEST_TEXTURE_HEIGHT = 128;
	constexpr i32 TEST_TEXTURE_PERIOD = 16;

	constexpr i32 WORK_X = 16;
	constexpr i32 WORK_Y = 16;

	constexpr i32 WX = 2047;
	constexpr i32 WY = 2047;
	constexpr i32 STEPS = 2 * 2;
	constexpr i32 ITERS = 1;*/

	inline std::string NAME = "computations";
	inline std::string SHADER_FOLDER = "shaders/";

	App::App(const cfg::json& config)
	{
		//initWindow();
	}

	App::~App()
	{
		//deinitWindow();
	}

	void App::mainloop()
	{
		// mainloop
		//m_mainWindow->show();
		//while (!m_mainWindow->shouldClose()) {
		//	glfw::poll_events();

		//	// rendering
		//	glClearColor(1.0, 0.1, 0.1, 1.0);
		//	glClearDepth(1.0);
		//	glClearStencil(0);

		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//	// swap
		//	m_mainWindow->swapBuffers();
		//}
	}

	/*void App::initWindow()
	{
		if (!glfw::initialize()) {
			throw std::runtime_error("Failed to initialize library.");
		}

		glfw::CreationInfo info;
		info.width  = WIDTH;
		info.height = HEIGHT;
		info.title = NAME;
		info.intHints.push_back({glfw::Hint::Resizable, (int)glfw::Value::False});
		info.intHints.push_back({glfw::Hint::ContextVersionMajor, 4});
		info.intHints.push_back({glfw::Hint::ContextVersionMinor, 5});
		info.intHints.push_back({glfw::Hint::DoubleBuffer, (int)glfw::Value::True});

		m_mainWindow = std::make_unique<win::MainWindow>(info);
		if (!m_mainWindow || !m_mainWindow->valid()) {
			throw std::runtime_error("Failed to create window");
		}
	}

	void App::deinitWindow()
	{
		m_mainWindow.reset();

		glfw::terminate();
	}*/
}
