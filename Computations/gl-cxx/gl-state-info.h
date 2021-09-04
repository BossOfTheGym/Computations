#pragma once

#include <string>
#include <ostream>

#include "gl-types.h"

namespace gl
{
	struct GlStateInfo
	{
		void acquireInfo();

		const GLubyte* vendor() const;

		const GLubyte* renderer() const;

		const GLubyte* version() const;

		void printAll(std::ostream& out);


		GLint majorVersion{};
		GLint minorVersion{};

		GLint minMapBufferAlignment{};

		GLint maxUniformBufferBindings{};
		GLint uniformBufferOffsetAlignment{};

		GLint shaderStorageBufferOffsetAlignment{};

		GLint maxComputeShaderMemorySize{};
		GLint maxComputeShaderWorkgroupInvocations{};
		GLint maxComputeWorkgroupCount[3]{};
		GLint maxComputeWorkgroupSize[3]{};	
	};
}
