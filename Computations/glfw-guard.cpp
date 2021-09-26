#include "glfw-guard.h"

#include <glfw-cxx/glfw3.h>

#include <exception>

GlfwGuard::GlfwGuard()
{
	if (!glfw::initialize()) {
		throw std::runtime_eror("Failed to initialize glfw.");
	}
}

GlfwGuard::~GlfwGuard()
{
	glfw::terminate();
}