#pragma once

#include <memory>
#include <cmath>
#include <cassert>

#include "core.h"
#include "handle.h"
#include "handle-pool.h"
#include "storage.h"
#include "graphics-res-util.h"

namespace dir2d
{
	// Problem description
	// div(grad(u)) = f
	// u(boundary) = g
	// first coord is y(rows), second coord is x(cols) as everything is stored in row-major manner
	// texture is 'padded' with boundary conditions
	struct DomainAabb2D
	{
		f32 x0{};
		f32 x1{};
		f32 y0{};
		f32 y1{};
		f32 hx{};
		f32 hy{};
		i32 xSplit{};
		i32 ySplit{};
	};

	struct DataAabb2D
	{
		DataAabb2D(i32 xExt, i32 yExt)
		{
			reset(xExt, yExt);
		}

		DataAabb2D() = default;
		~DataAabb2D() = default;

		DataAabb2D(const DataAabb2D&) = delete;
		DataAabb2D(DataAabb2D&&) noexcept = default;

		DataAabb2D& operator = (const DataAabb2D&) = delete;
		DataAabb2D& operator = (DataAabb2D&&) noexcept = default;


		void reset(i32 xExt, i32 yExt)
		{
			data.reset(new f32[xExt * yExt]);
		}

		f32* get() const
		{
			return data.get();
		}


		std::unique_ptr<f32[]> data;
	};


	class MeasureTime
	{
	public:
		struct TimeQuery
		{
			res::Query query{};
			GLuint64 dt{};
		};


	public:
		void addTimeQuery(Handle handle)
		{
			m_timeQueries.add(handle);

			auto& query = m_timeQueries.get(handle);
			assert(res::try_create_query(query.query));
		}

		void removeTimeQuery(Handle handle)
		{
			m_timeQueries.remove(handle);
		}

		TimeQuery& getTimeQuery(Handle handle)
		{
			return m_timeQueries.get(handle);
		}

		bool hasTimeQuery(Handle handle)
		{
			return m_timeQueries.has(handle);
		}


	private:
		Storage<TimeQuery> m_timeQueries;
	};

	template<i32 WORKGROUP_X_VAR, i32 WORKGROUP_Y_VAR>
	class BasicMethod
	{
	public:
		static constexpr const i32 WORKGROUP_X = WORKGROUP_X_VAR;
		static constexpr const i32 WORKGROUP_Y = WORKGROUP_Y_VAR;


		static void create_domainAabb2D(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
		{
			DomainAabb2D domain{x0, x1, y0, y1};
			if ((xSplit + 1) % WORKGROUP_X != 0)
			{
				xSplit = (xSplit + 1) - (xSplit + 1) % WORKGROUP_X + WORKGROUP_X - 1;
			}
			if ((ySplit + 1) % WORKGROUP_Y != 0)
			{
				ySplit = (ySplit + 1) - (ySplit + 1) % WORKGROUP_Y + WORKGROUP_Y - 1;
			}
			domain.hx = (x1 - x0) / xSplit;
			domain.hy = (y1 - y0) / ySplit;
			domain.xSplit = xSplit;
			domain.ySplit = ySplit;
			return domain;
		}

		template<class Solution>
		static DataAabb2D create_dataAabb2D(const DomainAabb2D& domain, Solution&& solution)
		{
			auto& [x0, x1, y0, y1, hx, hy, xSplit, ySplit] = domain;

			DataAabb2D data(xSplit + 1, ySplit + 1);

			auto ptr = data.get();
			// y0 boundary
			for (i32 j = 0; j <= xSplit; j++)
			{
				f32 x = x0 + j * hx;

				*ptr++ = solution(x, y0);
			}
			for (i32 i = 1; i < ySplit; i++)
			{
				f32 y = y0 + i * hy;

				// x0 boundary
				*ptr++ = solution(x0, y);
				for (i32 j = 1; j < xSplit; j++)
				{
					f32 x = x0 + j * hx;

					// NOTE : precondition can be used here
					// condition(precondition)
					*ptr++ = 0.0;
				}
				// x1 boundary
				*ptr++ = solution(x1, y);
			}
			// y1 boundary
			for (i32 j = 0; j <= xSplit; j++)
			{
				f32 x = x0 + j * hx;

				*ptr++ = solution(x, y1);
			}

			return data;
		}


	protected:
		Handle acquire()
		{
			return m_handlePool.acquire();
		}

		void release(Handle handle)
		{
			m_handlePool.release(handle);
		}

		bool valid(Handle handle) const
		{
			return m_handlePool.valid(handle);
		}


	private:
		HandlePool m_handlePool;
	};

	// TODO : MeasureTime should inherit from Method, add method for updating distinct data units
	class Jacoby : public BasicMethod<16, 16>, public MeasureTime
	{
	public:
		static constexpr const i32 IMG_PREV = 0;
		static constexpr const i32 IMG_NEXT = 1;

		struct Data
		{
			DomainAabb2D domain{};

			res::Texture iteration[2]{};

			i32 prev{};
			i32 next{};

			i32 update{};
			i32 cycles{};

			GLuint xNumGroups{};
			GLuint yNumGroups{};
		};		


		using UnderlyingType = BasicMethod<16, 16>;

	private:
		struct Uniforms
		{
			void getLocations(const res::ShaderProgram& jacobyProgram)
			{
				prev = glGetUniformLocation(jacobyProgram.id, "prev");
				next = glGetUniformLocation(jacobyProgram.id, "next");
				x0 = glGetUniformLocation(jacobyProgram.id, "x0");
				y0 = glGetUniformLocation(jacobyProgram.id, "y0");
				hx = glGetUniformLocation(jacobyProgram.id, "hx");
				hy = glGetUniformLocation(jacobyProgram.id, "hy");
			}

			bool valid() const
			{
				return next != -1 && prev != -1
					&& x0 != -1 && y0 != -1
					&& hx != -1 && hy != -1;
			}


			GLint next{-1};
			GLint prev{-1};
			GLint x0{-1};
			GLint y0{-1};
			GLint hx{-1};
			GLint hy{-1};
		};


		// TODO : that's ugly to my opinion
		void initData(Data& data, const DomainAabb2D& domain, const DataAabb2D& data2D, i32 cycles)
		{
			i32 xVars = domain.xSplit + 1;
			i32 yVars = domain.ySplit + 1;

			data.domain = domain;
			data.prev = 0;
			data.next = 1;
			data.iteration[0] = res::create_texture(xVars, yVars, GL_R32F); glTextureSubImage2D(data.iteration[data.prev].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.get());
			data.iteration[1] = res::create_texture(xVars, yVars, GL_R32F); glTextureSubImage2D(data.iteration[data.next].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.get());
			data.cycles = cycles;
			data.xNumGroups = xVars / WORKGROUP_X;
			data.yNumGroups = yVars / WORKGROUP_Y;
		}

	public:
		void setupProgram(res::ShaderProgram&& program)
		{
			m_program = std::move(program);
			m_uniforms.getLocations(m_program);
		}

		bool programValid() const
		{
			// TODO : res::ShaderProgram isValid
			return m_uniforms.valid() && m_program.id != 0;
		}


		// NOTE : general scheme, preset -> update -> some post action(no post action here)
		void preset()
		{
			glUseProgram(m_program.id);
		}

		void update()
		{
			for (auto handle : m_dataStorage)
			{
				auto& data = get(handle);

				bool hasQuery = hasTimeQuery(handle);
				if (hasQuery)
				{
					auto& query = getTimeQuery(handle);

					glGetQueryObjectui64v(query.query.id, GL_QUERY_RESULT, &query.dt);

					glBeginQuery(GL_TIME_ELAPSED, query.query.id);
				}

				glBindImageTexture(IMG_PREV, data.iteration[data.prev].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMG_NEXT, data.iteration[data.next].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

				glUniform1f(m_uniforms.x0, data.domain.x0);
				glUniform1f(m_uniforms.y0, data.domain.y0);
				glUniform1f(m_uniforms.hx, data.domain.hx);
				glUniform1f(m_uniforms.hy, data.domain.hy);

				for (i32 i = 0; i < data.cycles; i++)
				{
					glUniform1i(m_uniforms.prev, data.prev);
					glUniform1i(m_uniforms.next, data.next);

					// TODO : indirect dispatch
					glDispatchCompute(data.xNumGroups, data.yNumGroups, 1);

					std::swap(data.prev, data.next);
				}

				if (hasQuery)
				{
					glEndQuery(GL_TIME_ELAPSED);
				}
			}
		}


		// TODO : this method is almost similar in all classes. try templating
		Handle create(const DomainAabb2D& domain, const DataAabb2D& data2D, i32 cycles)
		{
			Handle handle = UnderlyingType::acquire();

			m_dataStorage.add(handle);
			initData(m_dataStorage.get(handle), domain, data2D, cycles);

			return handle;
		}

		void destroy(Handle handle)
		{
			assert(m_dataStorage.has(handle) && UnderlyingType::valid(handle));

			m_dataStorage.remove(handle);
			UnderlyingType::release(handle);
		}

		bool valid(Handle handle)
		{
			return m_dataStorage.has(handle) && UnderlyingType::valid(handle);
		}

		Data& get(Handle handle)
		{
			assert(m_dataStorage.has(handle) && UnderlyingType::valid(handle));

			return m_dataStorage.get(handle);
		}


	private:
		res::ShaderProgram  m_program;
		Uniforms            m_uniforms;
		Storage<Data>       m_dataStorage;
		Storage<res::Query> m_timeQueries;
	};

	class RedBlack : public BasicMethod<16, 16>, public MeasureTime
	{
	public:
		static constexpr const i32 IMG = 0;

		static f32 compute_optimal_w(f32 hx, f32 hy, i32 xSplit, i32 ySplit)
		{
			const constexpr f32 PId2 = 3.14159265359 / 2;

			i32 sx = xSplit;
			i32 sy = ySplit;

			f32 hxhx = hx * hx;
			f32 hyhy = hy * hy;
			f32 H = hxhx + hyhy;
			f32 sinx = std::sin(PId2 / sx);
			f32 siny = std::sin(PId2 / sy);

			f32 delta = 2.0 * hxhx / H * sinx * sinx + 2.0 * hyhy / H * siny * siny;

			return 2.0 / (1.0 + std::sqrt(delta * (2.0 - delta)));
		}

		struct Data
		{
			DomainAabb2D domain{};

			res::Texture iteration{};

			f32 w{};

			i32 updates{};
			i32 cycles{};

			GLuint xNumGroups{};
			GLuint yNumGroups{};
		};

		using UnderlyingType = BasicMethod<16, 16>;

	protected:
		struct Uniforms
		{
			void getLocations(const res::ShaderProgram& redBlackProgram)
			{
				rb = glGetUniformLocation(redBlackProgram.id, "rb");
				w  = glGetUniformLocation(redBlackProgram.id, "w");
				x0 = glGetUniformLocation(redBlackProgram.id, "x0");
				y0 = glGetUniformLocation(redBlackProgram.id, "y0");
				hx = glGetUniformLocation(redBlackProgram.id, "hx");
				hy = glGetUniformLocation(redBlackProgram.id, "hy");
			}

			bool valid() const
			{
				return rb != -1 && w != -1 && x0 != -1 && y0 != -1 && hx != -1 && hy != -1;
			}

			GLint rb{-1};
			GLint w{-1};
			GLint x0{-1};
			GLint y0{-1};
			GLint hx{-1};
			GLint hy{-1};
		};


		// NOTE, TODO : ugly
		void initData(Data& data, const DomainAabb2D& domain, const DataAabb2D& data2D, i32 cycles)
		{
			i32 xVar = domain.xSplit + 1;
			i32 yVar = domain.ySplit + 1;

			data.domain = domain;
			data.iteration = res::create_texture(xVar, yVar, GL_R32F); glTextureSubImage2D(data.iteration.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.get());
			data.w = compute_optimal_w(domain.hx, domain.hy, domain.xSplit, domain.ySplit);
			data.cycles = cycles;
			data.xNumGroups = xVar / WORKGROUP_X;
			data.yNumGroups = yVar / WORKGROUP_Y;
		}


	public:
		// NOTE, TODO : this is similar in all methods
		void setupProgram(res::ShaderProgram&& program)
		{
			m_program = std::move(program);
			m_uniforms.getLocations(m_program);
		}

		bool programValid() const
		{
			// TODO : res::ShaderProgram valid
			return m_uniforms.valid() && m_program.id != 0;
		}


		void setup()
		{
			glUseProgram(m_program.id);
		}

		void update()
		{
			for (auto& handle : m_dataStorage)
			{
				auto& data = get(handle);

				bool hasQuery = hasTimeQuery(handle);
				if (hasQuery)
				{
					auto& query = getTimeQuery(handle);

					glGetQueryObjectui64v(query.query.id, GL_QUERY_RESULT, &query.dt);

					glBeginQuery(GL_TIME_ELAPSED, query.query.id);
				}

				glBindImageTexture(IMG, data.iteration.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

				glUniform1f(m_uniforms.w, data.w);
				glUniform1f(m_uniforms.x0, data.domain.x0);
				glUniform1f(m_uniforms.y0, data.domain.y0);
				glUniform1f(m_uniforms.hx, data.domain.hx);
				glUniform1f(m_uniforms.hy, data.domain.hy);

				for (i32 i = 0; i < data.cycles; i++)
				{
					glUniform1i(m_uniforms.rb, 0);
					glDispatchCompute(data.xNumGroups, data.yNumGroups, 1);

					glUniform1i(m_uniforms.rb, 1);
					glDispatchCompute(data.xNumGroups, data.yNumGroups, 1);
				}

				if (hasQuery)
				{
					glEndQuery(GL_TIME_ELAPSED);
				}
			}
		}


		// NOTE, TODO : this method is almost similar in all classes. try templating
		Handle create(const DomainAabb2D& domain, const DataAabb2D& data2D, i32 cycles)
		{
			Handle handle = UnderlyingType::acquire();

			m_dataStorage.add(handle);
			initData(m_dataStorage.get(handle), domain, data2D, cycles);

			return handle;
		}

		void destroy(Handle handle)
		{
			assert(m_dataStorage.has(handle) && UnderlyingType::valid(handle));

			m_dataStorage.remove(handle);
			UnderlyingType::release(handle);
		}

		bool valid(Handle handle)
		{
			return m_dataStorage.has(handle) && UnderlyingType::valid(handle);
		}

		Data& get(Handle handle)
		{
			assert(m_dataStorage.has(handle) && UnderlyingType::valid(handle));

			return m_dataStorage.get(handle);
		}


	protected:
		res::ShaderProgram m_program{};
		Uniforms           m_uniforms{};
		Storage<Data>      m_dataStorage;
	};



	// TODO : HUGE TODO SECTION
	// TODO
	// red-red-black-black
	class MirroredRedBlack : public BasicMethod<16, 16>, public MeasureTime
	{
	public:
		struct Data
		{};
	};

	// TODO
	// jacoby, small tile
	struct JacobySmallTile
	{
		struct Data
		{};
	};

	// TODO
	// red-black, small tile
	struct RedBlackSmallTile
	{
		struct Data
		{};
	};

	// TODO
	// red-red-black-black, small tile
	struct MirroredRedBlackSmallTile
	{
		struct Data
		{};
	};
}
