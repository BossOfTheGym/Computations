#pragma once

#include <core.h>
#include <gl-cxx/gl-res.h>

namespace dir2d
{
	class TimeQuery
	{
	public:
		// throws std::runtime_error if cannot create query object
		TimeQuery();

		void reset();
		void start();
		void end();

		// returns time in nanoseconds
		GLuint64 elapsed() const;
		f64 elapsedMean() const;

	private:
		gl::Query m_timeQuery;
		GLuint64   m_elapsed{};
		f64		   m_elapsedMean{};
		GLuint64   m_measurements{};
		bool       m_ready{true};
	};
}