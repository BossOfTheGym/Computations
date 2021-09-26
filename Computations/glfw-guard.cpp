#include "glfw-guard.h"

#include <glfw-cxx/glfw3.h>

#include <stdexcept>

GlfwGuard::GlfwGuard()
{
	if (!glfw::initialize()) {
		throw std::runtime_error("Failed to initialize glfw.");
	}
}

GlfwGuard::~GlfwGuard()
{
	glfw::terminate();
}