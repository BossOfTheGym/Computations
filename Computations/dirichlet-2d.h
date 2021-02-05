#pragma once
#pragma once

#include <cassert>
#include <cmath>
#include <memory>
#include <algorithm>

#include "core.h"
#include "handle.h"
#include "handle-pool.h"
#include "storage.h"
#include "graphics-res-util.h"
#include "app-system.h"


namespace dir2d
{
	// TODO : entt::storage
	// TODO : indirect dispatch
	// TODO : change create_dataaabb method, xSplit, and ySplit must be xVars, yVars: resulting extents of a texture

	// Data accosiated with a given problem
	// div(grad(u)) = f
	// u(boundary) = g
	// first coord is y(rows), second coord is x(cols) as everything is stored in row-major manner
	// texture is 'padded' with boundary conditions
	// stores f32 data for single-channel texture
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
		f32* get() const
		{
			return data.get();
		}

		DomainAabb2D domain;

		std::unique_ptr<f32[]> data;
	};

	// manages handles
	class HandleProvider
	{
	public:
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

	// ensures that generated domain description will be correct
	template<i32 WORKGROUP_X, i32 WORKGROUP_Y>
	class DataAabbProvider
	{
	public:
		static auto create_dataAabb2D(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
		{
			DataAabb2D data{x0, x1, y0, y1};
			if ((xSplit + 1) % WORKGROUP_X != 0)
			{
				xSplit = (xSplit + 1) - (xSplit + 1) % WORKGROUP_X + WORKGROUP_X - 1;
			}
			if ((ySplit + 1) % WORKGROUP_Y != 0)
			{
				ySplit = (ySplit + 1) - (ySplit + 1) % WORKGROUP_Y + WORKGROUP_Y - 1;
			}
			f32 hx = (x1 - x0) / xSplit;
			f32 hy = (y1 - y0) / ySplit;

			data.domain.hx = hx;
			data.domain.hy = hy;
			data.domain.xSplit = xSplit;
			data.domain.ySplit = ySplit;

			// domain initialization
			data.data.reset(new f32[(xSplit + 1) * (ySplit + 1)]);

			auto ptr = data.get();
			// y0 boundary
			for (i32 j = 0; j <= xSplit; j++)
			{
				f32 x = x0 + j * hx;

				*ptr++ = std::exp(-x * x - y0 * y0);
			}
			for (i32 i = 1; i < ySplit; i++)
			{
				f32 y = y0 + i * hy;

				// x0 boundary
				*ptr++ = std::exp(-x0 * x0 - y * y);
				for (i32 j = 1; j < xSplit; j++)
				{
					f32 x = x0 + j * hx;

					// NOTE : precondition can be used here
					// condition(precondition)
					*ptr++ = 0.0;
				}
				// x1 boundary
				*ptr++ = std::exp(-x1 * x1 - y * y);
			}
			// y1 boundary
			for (i32 j = 0; j <= xSplit; j++)
			{
				f32 x = x0 + j * hx;

				*ptr++ = std::exp(-x * x - y1 * y1);
			}

			return data;
		}
	};

	// basic method, holds all neccessary data, manages required resources
	// MethodTraits must have:
	//  1. consts: WORKGROUP_X, WORKGROUP_Y
	//  2. types: Data, Uniforms
	template<class MethodTraits>
	class BasicMethod 
		: public app::System
		, public DataAabbProvider<MethodTraits::WORKGROUP_X, MethodTraits::WORKGROUP_Y>
		, protected HandleProvider
	{
	private:
		using Data = typename MethodTraits::Data;
		using Uniforms = typename MethodTraits::Uniforms;


	public:
		BasicMethod(app::App& app, res::ShaderProgram&& program) : app::System(app)
		{
			setupProgram(std::move(program));
		}


	public: // shader program & uniforms
		void setupProgram(res::ShaderProgram&& program)
		{
			m_program = std::move(program);
			m_uniforms.getLocations(m_program);
		}

		bool programValid() const
		{
			return m_uniforms.valid() && m_program.valid();
		}


	public: // data manipulation
		template<class ... Args>
		Handle create(Args&& ... args)
		{
			auto handle = acquire();

			m_dataStorage.emplace(handle, std::forward<Args>(args)...);

			return handle;
		}

		// destroy
		void destroy(Handle handle)
		{
			m_dataStorage.remove(handle);

			release(handle);
		}

		// valid
		bool valid(Handle handle)
		{
			return m_dataStorage.has(handle);
		}

	//protected:
		Data& get(Handle handle)
		{
			return m_dataStorage.get(handle);
		}

	protected:
		res::ShaderProgram m_program;
		Uniforms m_uniforms;
		Storage<Data> m_dataStorage;
	};


	// general traits that contain workgroup's size inforamtion: along x and y
	template<i32 WORKGROUP_X_VAR, i32 WORKGROUP_Y_VAR>
	struct WorkgroupTraits
	{
		static constexpr const i32 WORKGROUP_X = WORKGROUP_X_VAR;
		static constexpr const i32 WORKGROUP_Y = WORKGROUP_Y_VAR;
	};
	

	// traits specially for methods of jacoby family
	template<i32 WORKGROUP_X, i32 WORKGROUP_Y>
	class JacobyTraits : public WorkgroupTraits<WORKGROUP_X, WORKGROUP_Y>
	{
	private:
		friend class BasicMethod<JacobyTraits>;
		friend class JacobyMethod;

		struct Data
		{
			Data() = default;
			Data(const Data&) = delete; // implicitly deleted because of res::Texture
			Data(Data&&) = default;

			Data& operator = (const Data&) = delete; // implicitly deleted because of res::Texture
			Data& operator = (Data&&) = default;

			Data(const DataAabb2D& data2D, i32 cycles)
			{
				domain = data2D.domain;

				i32 xVars = domain.xSplit + 1;
				i32 yVars = domain.ySplit + 1;

				prev = 0;
				next = 1;

				iteration[0] = res::create_texture(xVars, yVars, GL_R32F); glTextureSubImage2D(iteration[prev].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.get());
				iteration[1] = res::create_texture(xVars, yVars, GL_R32F); glTextureSubImage2D(iteration[next].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.get());

				this->cycles = cycles;
				xNumGroups = xVars / WORKGROUP_X;
				yNumGroups = yVars / WORKGROUP_Y;
			}

			DomainAabb2D domain{};

			res::Texture iteration[2]{};

			i32 prev{};
			i32 next{};

			i32 update{};
			i32 cycles{};

			GLuint xNumGroups{};
			GLuint yNumGroups{};
		};		

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
	};

	// jacoby method impl
	class JacobyMethod : public BasicMethod<JacobyTraits<16, 16>>
	{
	private:
		using UnderlyingType = BasicMethod<JacobyTraits<16, 16>>;

		static constexpr const i32 IMG_PREV = 0;
		static constexpr const i32 IMG_NEXT = 1;

	public:
		JacobyMethod(app::App& app, res::ShaderProgram&& program) : UnderlyingType(app, std::move(program))
		{}

	public: // NOTE : general scheme, preset -> update -> some post action(no post action here)
		void setup()
		{
			glUseProgram(m_program.id);
		}

		// TODO : direct access to members of BasicMethod, protected access modifier is workaround. This needs to be fixed.
		void update()
		{
			for (auto handle : m_dataStorage)
			{
				auto& data = get(handle);

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

					glDispatchCompute(data.xNumGroups, data.yNumGroups, 1);

					std::swap(data.prev, data.next);
				}
			}
		}
	};


	// traits specially for methods of red-black family
	template<i32 WORKGROUP_X, i32 WORKGROUP_Y>
	class RedBlackTraits : WorkgroupTraits<WORKGROUP_X, WORKGROUP_Y>
	{
	private:
		friend class RedBlackMethod;
		friend class BasicMethod<RedBlackTraits>;

		struct Data
		{
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

			Data() = default;

			Data(const DataAabb2D& data2D, i32 cycles)
			{
				domain = data2D.domain;

				i32 xVar = domain.xSplit + 1;
				i32 yVar = domain.ySplit + 1;

				iteration = res::create_texture(xVar, yVar, GL_R32F); glTextureSubImage2D(iteration.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.get());

				w = compute_optimal_w(domain.hx, domain.hy, domain.xSplit, domain.ySplit);

				this->cycles = cycles;

				xNumGroups = xVar / WORKGROUP_X;
				yNumGroups = yVar / WORKGROUP_Y;
			}

			Data(const Data&) = delete;
			Data(Data&&) = default;

			Data& operator = (const Data&) = delete;
			Data& operator = (Data&&) = default;


			DomainAabb2D domain{};

			res::Texture iteration{};

			f32 w{};

			i32 updates{};
			i32 cycles{};

			GLuint xNumGroups{};
			GLuint yNumGroups{};
		};

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
	};

	// red-black method impl
	class RedBlackMethod : public BasicMethod<RedBlackTraits<16, 16>>
	{
	public:
		static constexpr const i32 IMG = 0;

		using UnderlyingType = BasicMethod<RedBlackTraits<16, 16>>;	

	public:
		RedBlackMethod(app::App& app, res::ShaderProgram&& program) : UnderlyingType(app, std::move(program))
		{}

	public:
		void setup()
		{
			glUseProgram(m_program.id);
		}

		// TODO : direct access to members of BasicMethod, protected access modifier is workaround. This needs to be fixed.
		void update()
		{
			for (auto& handle : m_dataStorage)
			{
				auto& data = get(handle);

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
			}
		}
	};
}
