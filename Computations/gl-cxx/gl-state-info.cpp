#include "gl-state-info.h"
#include "gl-header.h"

namespace gl
{
	void GlStateInfo::acquireInfo()
	{
		glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

		glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &minMapBufferAlignment);

		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffsetAlignment);

		glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &shaderStorageBufferOffsetAlignment);

		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxComputeShaderMemorySize);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxComputeShaderWorkgroupInvocations);

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, maxComputeWorkgroupCount);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, maxComputeWorkgroupCount + 1);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, maxComputeWorkgroupCount + 2);

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, maxComputeWorkgroupSize);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, maxComputeWorkgroupSize + 1);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, maxComputeWorkgroupSize + 2);
	}

	const GLubyte* GlStateInfo::vendor() const
	{
		return glGetString(GL_VENDOR);
	}

	const GLubyte* GlStateInfo::renderer() const
	{
		return glGetString(GL_RENDERER);
	}

	const GLubyte* GlStateInfo::version() const
	{
		return glGetString(GL_VERSION);
	}

	void GlStateInfo::printAll(std::ostream& out)
	{
		out << "*** OpenGL context info ***" << std::endl;
		out << "major version: " << majorVersion << std::endl;
		out << "minor version: " << minorVersion << std::endl;
		out << "version string: " << version() << std::endl;
		out << "vendor: " << vendor() << std::endl;
		out << "renderer: " << renderer() << std::endl;
		out << std::endl;
		out << "min map buffer alignment: " << minMapBufferAlignment << std::endl;
		out << std::endl;
		out << "max uniform buffer bindings: " << maxUniformBufferBindings << std::endl;
		out << "uniform buffer offsetAlignment: " << uniformBufferOffsetAlignment << std::endl;
		out << std::endl;
		out << "shader storage buffer offset alignment: " << shaderStorageBufferOffsetAlignment << std::endl;
		out << std::endl;
		out << "max compute shader memory size: " << maxComputeShaderMemorySize << std::endl;
		out << "max compute shader workgroup invocations: " << maxComputeShaderWorkgroupInvocations << std::endl;
		out << "max compute workgroup count x: " << maxComputeWorkgroupCount[0] << std::endl;
		out << "max compute workgroup count y: " << maxComputeWorkgroupCount[1] << std::endl;
		out << "max compute workgroup count z: " << maxComputeWorkgroupCount[2] << std::endl;
		out << "max compute workgroup size x:" << maxComputeWorkgroupSize[0] << std::endl;
		out << "max compute workgroup size y:" << maxComputeWorkgroupSize[1] << std::endl;
		out << "max compute workgroup size z:" << maxComputeWorkgroupSize[2] << std::endl;
	}
}
