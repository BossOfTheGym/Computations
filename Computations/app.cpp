#include "app.h"

#include <exception>

namespace app
{
	App::App() 
	{
		initWindow();
		initGraphicalResources();
		initSystems();
	}

	App::~App()
	{
		deinitSystems();
		deinitGraphicalResources();
		deinitWindow();
	}

	void App::mainloop()
	{
		// ***TEST***
		entt::scoped_connection connection{m_mainWindow->keyPressSink().connect<&App::updateSystemsCallback>(this)};
		// ***END TEST***k

		// mainloop
		m_mainWindow->show();
		while (!m_mainWindow->shouldClose()) {
			glfw::poll_events();

			if (!m_paused) {
				updateSystems();
			}

			// rendering
			glClearColor(1.0, 0.1, 0.1, 1.0);
			glClearDepth(1.0);
			glClearStencil(0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glUseProgram(m_shaderPrograms["quad"].id);

			int dw = WIDTH / 2;
			glViewport(0, 0, dw, HEIGHT);
			glBindVertexArray(m_dummy.id);
			glBindTextureUnit(0, m_handles[1].textureId());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(dw, 0, dw, HEIGHT);
			glBindVertexArray(m_dummy.id);
			glBindTextureUnit(0, m_handles[2].textureId());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			// swap
			m_mainWindow->swapBuffers();
		}
	}

	void App::updateSystemsCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_U && action == GLFW_PRESS) {
			updateSystems();
		}
		if (key == GLFW_KEY_P && action == GLFW_PRESS) {
			m_paused = !m_paused;

			std::cout << "paused: " << m_paused << "\n";
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			m_mainWindow->shouldClose(true);
		}
	}

	void App::updateSystems()
	{
		auto& rb      = m_dirichletSystem->get<dir2d::RedBlackMethod>();
		auto& rbTiled = m_dirichletSystem->get<dir2d::RedBlackTiledMethod>();

		rb.update();
		rbTiled.update();

		std::cout << "red-black: " << rb.timeElapsedMean() / 1000000.0 << "ms \n";
		std::cout << "red-black tiled: " << rbTiled.timeElapsedMean() / 1000000.0 << "ms \n";
	}


	void App::initWindow()
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

	void App::initGraphicalResources()
	{
		m_mainWindow->makeContextCurrent();

		m_glStateInfo = std::make_unique<res::GlStateInfo>();
		m_glStateInfo->acquireInfo();
		m_glStateInfo->printAll(std::cout);
		std::cout << std::endl;

		std::cout << "*** SHADERS ***" << std::endl;
		for (auto& dirEntry : fs::recursive_directory_iterator(SHADER_FOLDER)) {
			loadShader(dirEntry.path());
		}
		std::cout << std::endl;

		// shader programs		
		std::cout << "*** PROGRAMS ***" << std::endl;

		std::vector<std::vector<const char*>> programData
		{
			{"quad", "quad.vert", "quad.frag"},
			{"test_compute", "test_compute.comp"},
			{"jacoby", "jacoby.comp"},
			{"red_black", "red_black.comp"},
			{"red_black_tiled0", "red_black_tiled0.comp"},
			{"red_black_tiled1", "red_black_tiled1.comp"},
			{"red_black_small_tile"}, {"red_black_small_tile.comp"}
		};

		for (auto& data : programData) {
			createProgram(data);
		}

		// dummy array
		if (!res::try_create_vertex_array(m_dummy)) {
			throw std::runtime_error("Failed to create dummy array.");
		}

		// test texture
		if (!res::try_create_test_texture(m_testTex, TEST_TEXTURE_WIDTH, TEST_TEXTURE_HEIGHT, TEST_TEXTURE_PERIOD)) {
			throw std::runtime_error("Failed to create test texture.");
		}
	}

	void App::initSystems()
	{
		m_dirichletSystem = std::make_unique<DirichletSystem>(*this);
		if (!m_dirichletSystem) {
			throw std::runtime_error("Failed to initialize dirichlet system.");
		}

		m_dirichletSystem->add<dir2d::JacobyMethod>(*this, 16, 16, "jacoby");
		if (!m_dirichletSystem->has<dir2d::JacobyMethod>() || !m_dirichletSystem->get<dir2d::JacobyMethod>().programValid()) {
			throw std::runtime_error("Failed to initialize jacoby system");
		}

		m_dirichletSystem->add<dir2d::RedBlackMethod>(*this, WORK_X, WORK_Y, "red_black");
		if (!m_dirichletSystem->has<dir2d::RedBlackMethod>() || !m_dirichletSystem->get<dir2d::RedBlackMethod>().programValid()) {
			throw std::runtime_error("Failed to initialize red-black system");
		}

		m_dirichletSystem->add<dir2d::RedBlackTiledMethod>(*this, WORK_X, WORK_Y, "red_black_tiled1");
		if (!m_dirichletSystem->has<dir2d::RedBlackTiledMethod>() || !m_dirichletSystem->get<dir2d::RedBlackTiledMethod>().programValid()) {
			throw std::runtime_error("Failed to initialize red-black tiled system");
		}

		auto boundary = [] (f32 x, f32 y) -> f32
		{
			return std::exp(-x * x - y * y);
		};

		auto f = [] (f32 x, f32 y) -> f32
		{
			f32 xxpyy = x * x + y * y;

			return 4.0 * (xxpyy - 1) * std::exp(-xxpyy);
		};

		// TEST
		{
			auto& sys = m_dirichletSystem->get<dir2d::RedBlackMethod>();
			auto domain = sys.createDomain(-1.0, +1.0, -1.0, +1.0, WX, WY);
			auto data = sys.createData(domain, boundary, f);
			m_handles[1] = sys.createSmart(data, STEPS * ITERS);
		}
		// TEST
		{
			auto& sys = m_dirichletSystem->get<dir2d::RedBlackTiledMethod>();
			auto domain = sys.createDomain(-1.0, +1.0, -1.0, +1.0, WX, WY);
			auto data = sys.createData(domain, boundary, f);
			m_handles[2] = sys.createSmart(data, ITERS);
		}
	}


	void App::deinitWindow()
	{
		m_mainWindow.reset();

		glfw::terminate();
	}

	void App::deinitGraphicalResources()
	{
		m_shaders.clear();
		m_shaderPrograms.clear();

		m_glStateInfo.reset();
	}

	void App::deinitSystems()
	{		
		for (auto& handle : m_handles) {
			handle.reset();
		}
		m_dirichletSystem.reset();
	}
	

	void App::loadShader(const fs::path& path)
	{
		auto shaderPathStr = path.string();
		auto shaderName = shaderPathStr.substr(SHADER_FOLDER.size());

		res::Shader shader{};

		GLenum type = res::shader_type_from_extension(path);
		if (type != -1 && try_create_shader_from_file(shader, type, path)) {
			std::cout << "Loaded shader " << std::quoted(shaderPathStr) << std::endl;

			m_shaders[shaderName] = std::move(shader);
		}
		else {
			std::cerr << "Failed to load " << std::quoted(shaderPathStr) << std::endl;
		}
	}

	void App::createProgram(const std::vector<const char*>& data)
	{
		// data : prog_name, (shader_i, ...)
		if (data.empty()) {
			std::cerr << "Invalid data found while attempting to create shader program." << std::endl;
			return;
		}

		auto programName = data[0];

		std::vector<res::Shader*> requiredShaders;
		for (i32 i = 1; i < data.size(); i++) {
			if (auto it = m_shaders.find(data[i]); it != m_shaders.end()) {
				requiredShaders.push_back(&it->second);
			}
			else {
				std::cerr << "Failed to find shader " << std::quoted(data[i]) << std::endl;

				return;
			}
		}

		if (requiredShaders.size() + 1 != data.size()) {
			std::cerr << "Failed to create program " << std::quoted(programName) << std::endl;
			return;
		}

		res::ShaderProgram program;
		if (try_create_shader_program(program, requiredShaders.data(), requiredShaders.size())) {
			std::cout << std::quoted(programName) << " program created." << std::endl;

			m_shaderPrograms[programName] = std::move(program);
		}
		else {
			std::cerr << "Failed to create " << std::quoted(programName) << " shader program." << std::endl;
		}
	}

	res::Id App::getProgramId(const std::string& program)
	{
		if (auto it = m_shaderPrograms.find(program); it != m_shaderPrograms.end()) {
			return it->second.id;
		}
		return res::null;
	}
}
