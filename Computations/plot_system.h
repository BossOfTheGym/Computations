#pragma once

#include "core.h"
#include "app-system.h"
#include "graphics-res.h"

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace app
{
	class App;

	class PlotSystem : public System
	{
	public:
		PlotSystem(App& app) : System(app)
		{
			// TODO
		}

	public:
		void update()
		{
			// TODO
		}

	private:
		glm::mat4 m_proj{};
		glm::mat4 m_view{};
		glm::mat4 m_model{};

		// buffer
		res::Buffer m_buffer;
		i32 m_verticesSegmentOffset{};
		i32 m_verticesSegmentSize{};

		i32 m_indicesSegmentOffset{};
		i32 m_indicesSegmentSize{};

		i32 m_uniformSegmentOffset{};
		i32 m_uniformSegmentSize{};
	};
}