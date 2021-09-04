#include "time_query.h"

#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>

#include <exception>

namespace dir2d
{
	TimeQuery::TimeQuery()
	{
		if (!gl::try_create_query(m_timeQuery)) {
			throw std::runtime_error("Failed to create time query.");
		}
	}

	void TimeQuery::reset()
	{
		 m_elapsed = 0;
		 m_elapsedTotal = 0;
		 m_measurements = 0;
		 m_ready = false;
	}

	void TimeQuery::start()
	{
		glGetQueryObjectui64v(m_timeQuery.id, GL_QUERY_RESULT, &m_elapsed);

		glBeginQuery(GL_TIME_ELAPSED, m_timeQuery.id);
		if (m_ready) {
			m_elapsedTotal += m_elapsed;
			++m_measurements;
		}
		m_ready = true;
	}

	void TimeQuery::end() 
	{
		glEndQuery(GL_TIME_ELAPSED);
	}

	GLuint64 TimeQuery::elapsed() const
	{
		return m_elapsed;
	}

	f64 TimeQuery::elapsedMean() const
	{
		return m_measurements != 0 ? (f64)m_elapsed / m_measurements : 0.0;
	}
}