#include "window-builder.h"

#include <string>
#include <cassert>

#include <cfg.h>

MainWindowPtr WindowBuilder::build(const cfg::json& config)
{
	constexpr int DEFAULT_HEIGHT = 512;
	constexpr int DEFAULT_WIDTH = 1024;

	glfw::CreationInfo info;
	if (config.contains("title")) {
		info.title = config["title"].get<std::string>();
	}
	else {
		return {};
	}

	if (config.contains("height")) {
		info.height = config["height"].get<int>();
	}
	else {
		info.height = DEFAULT_HEIGHT;
	}

	if (config.contains("width")) {
		info.width = config["width"].get<int>();
	}
	else {
		info.width = DEFAULT_WIDTH;
	}

	// TODO : hints from config
	// default hints
	info.intHints.push_back({glfw::Hint::Resizable, (int)glfw::Value::False});
	info.intHints.push_back({glfw::Hint::ContextVersionMajor, 4});
	info.intHints.push_back({glfw::Hint::ContextVersionMinor, 5});
	info.intHints.push_back({glfw::Hint::DoubleBuffer, (int)glfw::Value::True});

	auto window = std::make_unique<win::MainWindow>(info);
	window->makeContextCurrent();
	return window;
}
