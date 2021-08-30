#pragma once

#include <cmath>
#include <memory>
#include <cassert>
#include <algorithm>
#include <exception>

#include <core.h>
#include <handle.h>
#include <handle-pool.h>
#include <storage.h>
#include <graphics-res-util.h>
#include <app-system.h>


namespace dir2d
{
	// TODO : indirect dispatch
	// TODO : multiple programs per system(maybe)
	// TODO : attach WORKGROUP consts to shaders
	// TODO : template to check if Uniform type satisfies required conditions
	// TODO : template to check if Data type satisfies required condition
	// 
	// TODO : smart handle must return data bundle: domain info and texture id
	// 
	// TODO : refactor : shader program should not be part of method
	// TODO : refactor : decouple time measurement class
	// TODO : refactor split this shit up
	// 
	// Data accosiated with a given problem
	// div(grad(u)) = f
	// u(boundary) = g
	// first coord is y(rows), second coord is x(cols) as everything is stored in row-major manner
	// texture is 'padded' with boundary conditions
	// stores f32 data for single-channel texture
	using Function2D = std::function<f32(f32, f32)>;

	struct DomainAabb2D
	{
		static bool split_aligned(i32 split, i32 alignment)
		{
			return (split + 1) % alignment == 0;
		}

		static i32 align_split(i32 split, i32 alignment)
		{
			if (!split_aligned(split, alignment)) {
				split = (split + 1) - (split + 1) % alignment + alignment - 1;
			}

			return split;
		}

		static DomainAabb2D create_domain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
		{
			return DomainAabb2D{x0, x1, y0, y1, (x1 - x0) / xSplit, (y1 - y0) / ySplit, xSplit, ySplit};
		}

		static DomainAabb2D create_aligned_domain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit, i32 xAlign, i32 yAlign)
		{
			xSplit = align_split(xSplit, xAlign);
			ySplit = align_split(ySplit, yAlign);

			return DomainAabb2D{x0, x1, y0, y1, (x1 - x0) / xSplit, (y1 - y0) / ySplit, xSplit, ySplit};
		}

		static bool domain_aligned(const DomainAabb2D& domain, i32 xAlign, i32 yAlign)
		{
			return split_aligned(domain.xSplit, xAlign) && split_aligned(domain.ySplit, yAlign);
		}

		static void align_domain(DomainAabb2D& domain, i32 xAlign, i32 yAlign)
		{
			domain.xSplit = align_split(domain.xSplit, xAlign);
			domain.ySplit = align_split(domain.ySplit, yAlign);
		}


		f32 x0{};
		f32 x1{};
		f32 y0{};
		f32 y1{};
		f32 hx{};
		f32 hy{};
		i32 xSplit{};
		i32 ySplit{};
	};

	struct DataAabb2D : DomainAabb2D
	{
	private:
		static void initialize_solution_data(DataAabb2D& data, const Function2D& boundary)
		{
			data.solution.reset(new f32[(data.xSplit + 1) * (data.ySplit + 1)]);

			auto ptr = data.solution.get();
			for (i32 j = 0; j <= data.xSplit; j++) {
				f32 x = data.x0 + j * data.hx;

				*ptr++ = boundary(x, data.y0);				
			}
			for (i32 i = 1; i < data.ySplit; i++) {
				f32 y = data.y0 + i * data.hy;

				*ptr++ = boundary(data.x0, y);
				for (i32 j = 1; j < data.xSplit; j++) {
					f32 x = data.x0 + j * data.hx;

					*ptr++ = 0.0;
				}
				*ptr++ = boundary(data.x1, y);
			}
			for (i32 j = 0; j <= data.xSplit; j++) {
				f32 x = data.x0 + j * data.hx;

				*ptr++ = boundary(x, data.y1);
			}
		}

		static void initialize_f_data(DataAabb2D& data, const Function2D& f)
		{
			data.f.reset(new f32[(data.xSplit + 1) * (data.ySplit + 1)]);

			auto ptr = data.f.get();
			for (i32 j = 0; j <= data.xSplit; j++) {
				*ptr++ = 0.0f;
			}
			for (i32 i = 1; i < data.ySplit; i++)
			{
				f32 y = data.y0 + i * data.hy;

				*ptr++ = 0.0f;
				for (i32 j = 1; j < data.xSplit; j++) {
					f32 x = data.x0 + j * data.hx;

					*ptr++ = f(x, y);
				}
				*ptr++ = 0.0;
			}
			for (i32 j = 0; j <= data.xSplit; j++) {
				*ptr++ = 0.0f;
			}
		}

	public:
		static DataAabb2D create_data(const DomainAabb2D& domain, const Function2D& boundary, const Function2D& f)
		{
			DataAabb2D data{domain};

			initialize_solution_data(data, boundary);
			initialize_f_data(data, f);

			return data;
		}

		std::unique_ptr<f32[]> solution;
		std::unique_ptr<f32[]> f;
	};


	// ensures that generated domain description will be correct
	// smart handle, used to obtain height texture id
	// its lifetime should not exceed the lifetime of the system it belongs
	class SmartHandle
	{
		using DestroyFunc = void (*)(void*, Handle);
		using ObtainFunc = res::Id (*)(void*, Handle);

		template<class T>
		friend class BasicMethod;

		SmartHandle(Handle handle, void* instance, DestroyFunc destroy, ObtainFunc obtain)
			: m_handle{handle}
			, m_instance{instance}
			, m_destroyFunc{destroy}
			, m_obtainFunc{obtain}
		{}

	public:
		SmartHandle() = default;

		SmartHandle(const SmartHandle&) = delete;
		SmartHandle& operator = (const SmartHandle&) = delete;

		SmartHandle(SmartHandle&& another) noexcept
			: m_handle{std::exchange(another.m_handle, null)}
			, m_instance{std::exchange(another.m_instance, nullptr)}
			, m_obtainFunc{std::exchange(another.m_obtainFunc, nullptr)}
			, m_destroyFunc{std::exchange(another.m_destroyFunc, nullptr)}
		{}

		SmartHandle& operator = (SmartHandle&& another) noexcept
		{
			if (this == &another) {
				return *this;
			}

			reset();

			m_handle = std::exchange(another.m_handle, null);
			m_instance = std::exchange(another.m_instance, nullptr);
			m_destroyFunc = std::exchange(another.m_destroyFunc, nullptr);
			m_obtainFunc = std::exchange(another.m_obtainFunc, nullptr);

			return *this;
		}

		~SmartHandle()
		{
			reset();
		}


	public:
		void reset()
		{
			if (!empty()) {
				m_destroyFunc(m_instance, m_handle);

				m_handle = null;
				m_instance = nullptr;
				m_destroyFunc = nullptr;
				m_obtainFunc = nullptr;
			}
		}

		bool empty() const
		{
			return m_handle == null;
		}

		Handle handle() const
		{
			return m_handle;
		}

		res::Id textureId() const
		{
			return m_obtainFunc(m_instance, m_handle);
		}


	private:
		Handle m_handle{null};

		void* m_instance{};
		DestroyFunc m_destroyFunc{};
		ObtainFunc m_obtainFunc{};
	};

	struct BaseData : DomainAabb2D
	{
		BaseData() = default;

		BaseData(const DomainAabb2D& domain, i32 iterationsPerUpdate) : DomainAabb2D(domain), itersPerUpdate(iterationsPerUpdate)
		{}
	
		i32 itersPerUpdate{};
	};

	// basic method, holds all neccessary data, manages required resources
	// MethodTraits must have:
	//  1. types: Data(has textureId method), Uniforms(has getLocations method)
	//
	// All data types must be constructed from DataAabb2D structure(it always goes as first parameter)
	template<class MethodTraits>
	class BasicMethod : public app::System, protected HandlePool
	{
		using Data     = typename MethodTraits::Data;
		using Uniforms = typename MethodTraits::Uniforms;

		static void destroy(void* instance, Handle handle)
		{
			static_cast<BasicMethod*>(instance)->destroy(handle);
		}

		static res::Id obtain(void* instance, Handle handle)
		{
			return static_cast<BasicMethod*>(instance)->textureId(handle);
		}


	public:
		BasicMethod(app::App& app, u32 workgroupSizeX, u32 workgroupSizeY, const std::string& program) 
			: app::System(app)
			, m_workgroupSizeX{workgroupSizeX}
			, m_workgroupSizeY{workgroupSizeY}
		{
			setupProgram(program);

			m_timeQuery = res::create_query();
		}


	public: // workgroups
		u32 workgroupX() const
		{
			return m_workgroupSizeX;
		}

		u32 workgroupY() const
		{
			return m_workgroupSizeY;
		}


	public: // domain & data
		DomainAabb2D createDomain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
		{
			return DomainAabb2D::create_domain(x0, x1, y0, y1, xSplit, ySplit);
		}

		DomainAabb2D createAlignedDomain(f32 x0, f32 x1, f32 y0, f32 y1, i32 xSplit, i32 ySplit)
		{
			return DomainAabb2D::create_aligned_domain(x0, x1, y0, y1, xSplit, ySplit, m_workgroupSizeX, m_workgroupSizeY);
		}

		DataAabb2D createData(const DomainAabb2D& domain, const Function2D& boundary, const Function2D& f)
		{
			return DataAabb2D::create_data(domain, boundary, f);
		}


	public: // program & uniforms
		void setupProgram(const std::string& program)
		{
			m_program = program;

			updateUniforms();
		}

		void updateUniforms()
		{
			if (auto id = getShaderProgramId(); id != res::null) {
				m_uniforms.getLocations(id);
			}
		}

		res::Id getShaderProgramId() const
		{
			return app().getProgramId(m_program);
		}

		bool programValid() const
		{
			return m_uniforms.valid() && getShaderProgramId() != res::null;
		}


	public:
		GLuint64 timeElapsed() const
		{
			return m_elapsed;
		}

		f64 timeElapsedMean() const
		{
			return m_elapsedMean;
		}

	protected:
		void startTimeQuery()
		{
			glGetQueryObjectui64v(m_timeQuery.id, GL_QUERY_RESULT, &m_elapsed);

			glBeginQuery(GL_TIME_ELAPSED, m_timeQuery.id);

			if (!m_skipOnce) {
				m_elapsedMean = (m_elapsedMean * m_measurements + m_elapsed) / static_cast<f64>(m_measurements + 1);
				++m_measurements;
			}
			m_skipOnce = false;
		}

		void endTimeQuery()
		{
			glEndQuery(GL_TIME_ELAPSED);
		}


	public: // data storage
		template<class ... Args>
		Handle create(const DataAabb2D& data, Args&& ... args)
		{
			auto handle = acquire();

			m_dataStorage.emplace(handle, data, std::forward<Args>(args)...);

			return handle;
		}

		template<class ... Args>
		SmartHandle createSmart(Args&& ... args)
		{
			return SmartHandle{create(std::forward<Args>(args)...), this, destroy, obtain};
		}

		void destroy(Handle handle)
		{
			m_dataStorage.remove(handle);

			release(handle);
		}

		bool valid(Handle handle)
		{
			return m_dataStorage.has(handle);
		}

		res::Id textureId(Handle handle)
		{
			return get(handle).textureId();
		}

	protected:
		Data& get(Handle handle)
		{
			return m_dataStorage.get(handle);
		}


	protected:
		u32 m_workgroupSizeX{};
		u32 m_workgroupSizeY{};

		res::Query m_timeQuery;
		GLuint64   m_elapsed{};
		GLuint64   m_measurements{};
		f64        m_elapsedMean{};
		bool       m_skipOnce{true};

		std::string m_program;
		Uniforms    m_uniforms;

		Storage<Data> m_dataStorage;
	};


	// traits specially for methods of jacoby family
	struct JacobyTraits
	{
		struct Data : BaseData
		{
			Data() = default;

			Data(const Data&) = delete; // implicitly deleted because of res::Texture
			Data(Data&&) = default;

			Data& operator = (const Data&) = delete; // implicitly deleted because of res::Texture
			Data& operator = (Data&&) = default;

			Data(const DataAabb2D& data2D, i32 iterationsPerUpdate) : BaseData(data2D, iterationsPerUpdate)
			{
				i32 xVars = xSplit + 1;
				i32 yVars = ySplit + 1;

				iteration[0] = res::create_texture(xVars, yVars, GL_R32F);
				iteration[1] = res::create_texture(xVars, yVars, GL_R32F);
				glTextureSubImage2D(iteration[0].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.solution.get());
				glTextureSubImage2D(iteration[1].id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.solution.get());

				f = res::create_texture(xVars, yVars, GL_R32F);
				glTextureSubImage2D(f.id, 0, 0, 0, xVars, yVars, GL_RED, GL_FLOAT, data2D.f.get());
			}


			res::Id textureId() const
			{
				return iteration[0].id;
			}


		public:
			res::Texture iteration[2]{};
			res::Texture f{};
		};		

		struct Uniforms
		{
			void getLocations(res::Id jacobyProgram)
			{
				curr = glGetUniformLocation(jacobyProgram, "curr");
				hx = glGetUniformLocation(jacobyProgram, "hx");
				hy = glGetUniformLocation(jacobyProgram, "hy");
			}

			bool valid() const
			{
				return curr != -1 && hx != -1 && hy != -1;
			}

			GLint curr{-1};
			GLint hx{-1};
			GLint hy{-1};
		};
	};

	// jacoby method impl
	class JacobyMethod : public BasicMethod<JacobyTraits>
	{
		using UnderlyingType = BasicMethod<JacobyTraits>;

		static constexpr const u32 IMG0 = 0;
		static constexpr const u32 IMG1 = 1;
		static constexpr const u32 IMGF = 2;

	public:
		JacobyMethod(app::App& app, u32 workgroupSizeX, u32 workgroupSizeY, const std::string& program) 
			: UnderlyingType(app, workgroupSizeX, workgroupSizeY, program)
		{}

	public:
		void update()
		{
			glUseProgram(getShaderProgramId());

			startTimeQuery();

			for (auto handle : m_dataStorage)
			{
				auto& data = get(handle);

				glBindImageTexture(IMG0, data.iteration[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMG1, data.iteration[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMGF, data.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

				glUniform1f(m_uniforms.hx, data.hx);
				glUniform1f(m_uniforms.hy, data.hy);

				u32 numWorkgroupsX = data.xSplit / workgroupX() + 1;
				u32 numWorkgroupsY = data.ySplit / workgroupY() + 1;
				for (i32 i = 0; i < data.itersPerUpdate; i++)
				{
					glUniform1i(m_uniforms.curr, 0);

					glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);

					glUniform1i(m_uniforms.curr, 1);

					glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
				}
			}

			endTimeQuery();
		}
	};


	// traits specially for methods of red-black family
	struct RedBlackTraits
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

		struct Data : BaseData
		{
			Data() = default;

			Data(const DataAabb2D& data2D, i32 iterationsPerUpdate) : BaseData(data2D, iterationsPerUpdate)
			{
				i32 xVar = xSplit + 1;
				i32 yVar = ySplit + 1;

				iteration = res::create_texture(xVar, yVar, GL_R32F);
				glTextureSubImage2D(iteration.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.solution.get());

				f = res::create_texture(xVar, yVar, GL_R32F);
				glTextureSubImage2D(f.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.f.get());

				w = compute_optimal_w(hx, hy, xSplit, ySplit);
			}

			Data(const Data&) = delete;
			Data(Data&&) = default;

			Data& operator = (const Data&) = delete;
			Data& operator = (Data&&) = default;


			res::Id textureId() const
			{
				return iteration.id;
			}


			res::Texture iteration{};
			res::Texture f{};
			f32 w{};
		};

		struct Uniforms
		{
			void getLocations(res::Id redBlackProgram)
			{
				rb = glGetUniformLocation(redBlackProgram, "rb");
				w  = glGetUniformLocation(redBlackProgram, "w");
				hx = glGetUniformLocation(redBlackProgram, "hx");
				hy = glGetUniformLocation(redBlackProgram, "hy");
			}

			bool valid() const
			{
				return rb != -1 && w != -1 &&  hx != -1 && hy != -1;
			}

			GLint rb{-1};
			GLint w{-1};
			GLint hx{-1};
			GLint hy{-1};
		};
	};

	// red-black method impl
	class RedBlackMethod : public BasicMethod<RedBlackTraits>
	{
		static constexpr const u32 IMG = 0;
		static constexpr const u32 IMGF = 1;

		using UnderlyingType = BasicMethod<RedBlackTraits>;	

	public:
		RedBlackMethod(app::App& app, u32 workgroupSizeX, u32 workgroupSizeY, const std::string& program) 
			: UnderlyingType(app, workgroupSizeX, workgroupSizeY, program)
		{}

	public:
		void update()
		{
			glUseProgram(getShaderProgramId());

			startTimeQuery();

			for (auto& handle : m_dataStorage)
			{
				auto& data = get(handle);

				glBindImageTexture(IMG, data.iteration.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMGF, data.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

				glUniform1f(m_uniforms.w, data.w);
				glUniform1f(m_uniforms.hx, data.hx);
				glUniform1f(m_uniforms.hy, data.hy);

				u32 numWorkgroupsX = data.xSplit / workgroupX() + 1;
				u32 numWorkgroupsY = data.ySplit / workgroupY() + 1;
				for (i32 i = 0; i < data.itersPerUpdate; i++)
				{
					glUniform1i(m_uniforms.rb, 0);
					glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
					glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

					glUniform1i(m_uniforms.rb, 1);
					glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
					glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
				}
			}

			endTimeQuery();
		}
	};


	// traits for tiled method family
	struct RedBlackTiledTraits
	{
		struct Data : BaseData
		{
			Data() = default;

			Data(const DataAabb2D& data2D, i32 iterationsPerUpdate) : BaseData(data2D, iterationsPerUpdate)
			{
				i32 xVar = xSplit + 1;
				i32 yVar = ySplit + 1;

				iteration[0] = res::create_texture(xVar, yVar, GL_R32F);
				iteration[1] = res::create_texture(xVar, yVar, GL_R32F);
				glTextureSubImage2D(iteration[0].id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.solution.get());
				glTextureSubImage2D(iteration[1].id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.solution.get());

				f = res::create_texture(xVar, yVar, GL_R32F);
				glTextureSubImage2D(f.id, 0, 0, 0, xVar, yVar, GL_RED, GL_FLOAT, data2D.f.get());

				w = RedBlackTraits::compute_optimal_w(hx, hy, xSplit, ySplit);
			}

			Data(const Data&) = delete;
			Data(Data&&) = default;

			Data& operator = (const Data&) = delete;
			Data& operator = (Data&&) = default;


			res::Id textureId() const
			{
				return iteration[0].id;
			}


			res::Texture iteration[2]{};
			res::Texture f{};

			f32 w{};
		};

		struct Uniforms
		{
			void getLocations(res::Id redBlackProgram)
			{
				curr = glGetUniformLocation(redBlackProgram, "curr");
				w  = glGetUniformLocation(redBlackProgram, "w");
				hx = glGetUniformLocation(redBlackProgram, "hx");
				hy = glGetUniformLocation(redBlackProgram, "hy");
			}

			bool valid() const
			{
				return curr != -1 && w != -1 && hx != -1 && hy != -1;
			}

			GLint curr{-1};
			GLint w{-1};
			GLint hx{-1};
			GLint hy{-1};
		};
	};

	class RedBlackTiledMethod : public BasicMethod<RedBlackTiledTraits>
	{
		using UnderlyingType = BasicMethod<RedBlackTiledTraits>;

		static constexpr const u32 IMG0 = 0;
		static constexpr const u32 IMG1 = 1;
		static constexpr const u32 IMGF = 2;

	public:
		RedBlackTiledMethod(app::App& app, u32 workgroupSizeX, u32 workgroupSizeY, const std::string& program) 
			: UnderlyingType(app, workgroupSizeX, workgroupSizeY, program)
		{}

	public:
		void update()
		{
			glUseProgram(getShaderProgramId());

			startTimeQuery();

			for (auto& handle : m_dataStorage)
			{
				auto& data = get(handle);

				glBindImageTexture(IMG0, data.iteration[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMG1, data.iteration[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMGF, data.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

				glUniform1f(m_uniforms.w, data.w);
				glUniform1f(m_uniforms.hx, data.hx);
				glUniform1f(m_uniforms.hy, data.hy);

				u32 numWorkgroupsX = data.xSplit / workgroupX() + 1;
				u32 numWorkgroupsY = data.ySplit / workgroupY() + 1;
				for (i32 i = 0; i < data.itersPerUpdate; i++)
				{
					glUniform1i(m_uniforms.curr, 0);
					glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
					glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

					glUniform1i(m_uniforms.curr, 1);
					glDispatchCompute(numWorkgroupsX, numWorkgroupsY, 1);
					glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
				}
			}

			endTimeQuery();
		}
	};


	// traits for small tile (smt) family
	struct RebBlackSmtTraits : protected RedBlackTiledTraits
	{
		using Base = RedBlackTiledTraits;

		using Data = RedBlackTiledTraits::Data;
		
		struct Uniforms : Base::Uniforms
		{
			void getLocations(res::Id redBlackProgram)
			{
				Base::Uniforms::getLocations(redBlackProgram);
				numWorkgroupsX = glGetUniformLocation(redBlackProgram, "numWorkgroupsX");
				numWorkgroupsY = glGetUniformLocation(redBlackProgram, "numWorkgroupsY");
			}

			bool valid() const
			{
				return Base::Uniforms::valid() && numWorkgroupsX != -1 && numWorkgroupsY != -1;	
			}

			GLint numWorkgroupsX{-1};
			GLint numWorkgroupsY{-1};
		};
	};

	class RedBlackSmtMethod : public BasicMethod<RebBlackSmtTraits>
	{
		using UnderlyingType = BasicMethod<RebBlackSmtTraits>;

		static constexpr const u32 IMG0 = 0; // solution[0]
		static constexpr const u32 IMG1 = 1; // solution[1]
		static constexpr const u32 IMGF = 2; // f
		static constexpr const u32 IMGI = 3; // intermediate

	public:
		RedBlackSmtMethod(app::App& app, u32 workgroupSizeX, u32 workgroupSizeY, const std::string& program)
			: UnderlyingType(app, workgroupSizeX, workgroupSizeY, program)
		{}

	public:
		void update()
		{
			glUseProgram(getShaderProgramId());

			startTimeQuery();

			for (auto& handle : m_dataStorage) {
				auto& data = get(handle);

				glBindImageTexture(IMG0, data.iteration[0].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMG1, data.iteration[1].id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
				glBindImageTexture(IMGF, data.f.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

				glUniform1f(m_uniforms.w, data.w);
				glUniform1f(m_uniforms.hx, data.hx);
				glUniform1f(m_uniforms.hy, data.hy);

				u32 numWorkgroupsX = data.xSplit / workgroupX() + 1;
				u32 numWorkgroupsY = data.ySplit / workgroupY() + 1;
				
				// TODO
				// TODO : barriers : txture & buffer
			}

			endTimeQuery();
		}
	};
}
