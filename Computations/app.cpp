#include "app.h"

#include <glfw-cxx/glfw3.h>

#include <gl-cxx/gl-res.h>
#include <gl-cxx/gl-header.h>
#include <gl-cxx/gl-res-util.h>
#include <gl-cxx/gl-state-info.h>

#include <modules/module.h>
#include <modules/bundle.h>

#include <dirichlet/dirichlet-proxy.h>
#include <dirichlet/dirichlet_handle.h>
#include <dirichlet/dirichlet_dataaabb2d.h>
#include <dirichlet/dirichlet_domainaabb2d.h>
#include <dirichlet/dirichlet_cfg.h>

#include <fs.h>
#include <cfg.h>
#include <core.h>
#include <grid.h>
#include <metainfo.h>
#include <app-params.h>
#include <main-window.h>
#include <output-params.h>
#include <window-builder.h>
#include <shader-storage.h>
#include <program-storage.h>

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace app
{
	namespace detail
	{
		using namespace cfg;
		using namespace dir2d;

		class Tracker
		{
		public:
			void trackElapsed(GLint64 t)
			{
				m_elapsed.push_back(t);
			}

			void trackElapsedMean(f64 t)
			{
				m_elapsedMean.push_back(t);
			}

			json toJson() const
			{
				json result;
				result["elapsed"] = m_elapsed;
				result["elapsed_mean"] = m_elapsedMean;
				return result;
			}

		private:
			std::vector<GLint64> m_elapsed;
			std::vector<f64> m_elapsedMean;
		};

		struct RequiredModules
		{
			static RequiredModules get(Module& root)
			{
				return RequiredModules{
					.window         = root.acquire("window"),
					.programStorage = root.acquire("program_storage"),
					.dirichletProxy = root.acquire("dirichlet")->acquire("controls"),
					.app            = root.acquire("app"),
					.output         = root.acquire("output"),
					.grid           = root.acquire("grid"),
					.metainfo       = root.acquire("metainfo"),
				};
			}

			ModulePtr window;
			ModulePtr programStorage;
			ModulePtr dirichletProxy;
			ModulePtr app;
			ModulePtr output;
			ModulePtr grid;
			ModulePtr metainfo;
		};

		struct InitData
		{
			static InitData get(uint xSplit, uint ySplit)
			{
				auto boundary = [] (f32 x, f32 y) -> f32
				{
					return std::exp(-x * x - y * y);
				};

				auto f = [] (f32 x, f32 y) -> f32
				{
					f32 xxpyy = x * x + y * y;
					return 4.0 * (xxpyy - 1) * std::exp(-xxpyy);
				};

				InitData initData;
				initData.domain = DomainAabb2D::create_domain(-1.0, 1.0, -1.0, 1.0, xSplit, ySplit);
				initData.data   = DataAabb2D::create_data(initData.domain, boundary, f);
				return initData;
			}

			DomainAabb2D domain;
			DataAabb2D data;
		};


		class AppImpl
		{
		public:
			AppImpl(const json& config)
			{
				m_bundle = std::make_unique<Bundle>(config);
			}

			void mainloop()
			{
				auto requiredModules = RequiredModules::get(m_bundle->root());
				auto& appParams = requiredModules.app->get<AppParams>();
				auto& grid      = requiredModules.grid->get<Grid>();
				auto& window    = requiredModules.window->get<MainWindowPtr>();

				uint updates = requiredModules.app->get<AppParams>().totalUpdates;

				std::vector<SmartHandle> handles = createHandles(appParams.xSplit, appParams.ySplit, requiredModules.dirichletProxy);

				printProxyOrder(requiredModules.dirichletProxy);

				std::unordered_map<std::string, Tracker> trackers;
				while (updates-- > 0 && !window->shouldClose())
				{
					glfw::poll_events();
					window->swapBuffers();
					
					glClearColor(0.5, 0.5, 0.5, 1.0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					for (auto& [name, ptr] : *requiredModules.dirichletProxy) {
						auto& tracker = trackers[name];

						auto& proxy = ptr->get<Proxy>();
						proxy.update();
						tracker.trackElapsed(proxy.elapsed());
						tracker.trackElapsedMean(proxy.elapsedMean());
					}

					grid.setup();
					{
						uint index = 0;
						for (auto& handle : handles) {
							grid.render(handle.texture(), index++);
						}
					}
				}

				json data;
				data["data"] = createTrackerOutput(trackers.begin(), trackers.end());
				data["meta"] = createMetadata(requiredModules.metainfo);
				writeOutput(data, requiredModules.output);
			}

		private:
			void printProxyOrder(ModulePtr proxies)
			{
				std::cout << "Proxy order : ";
				for (auto& [name, ptr] : *proxies) {
					std::cout << name << " ";
				}
				std::cout << "\n";
			}

			std::vector<SmartHandle> createHandles(uint xSplit, uint ySplit, ModulePtr proxies)
			{
				auto initData = InitData::get(xSplit, ySplit);

				std::vector<SmartHandle> handles;
				for (auto& [name, ptr] : *proxies) {
					auto& proxy = ptr->get<Proxy>();
					handles.push_back(proxy.createSmart(initData.domain, initData.data, {1}));
				}
				return handles;
			}

			template<class It>
			json createTrackerOutput(It first, It last)
			{
				json output;
				while (first != last) {
					auto& [name, tracker] = *first;
					output[name] = tracker.toJson();
					++first;
				}
				return output;
			}

			json createMetadata(ModulePtr metainfo)
			{
				gl::GlStateInfo info;

				json meta = metainfo->get<Metainfo>().metainfo;
				meta["renderer"] = (const char*)info.renderer();
				return meta;
			}

			void writeOutput(const json& data, ModulePtr output)
			{
				fs::path path = output->get<OutputParams>().output;

				if (path.has_parent_path()) {
					std::error_code ec;
					if (fs::create_directories(path.parent_path(), ec); ec) {
						throw std::runtime_error("Failed to create output directories.");
					}
				}

				std::ofstream outputStream(path);
				if (!outputStream) {
					throw std::runtime_error("Failed to open/create output file.");
				}
				outputStream << std::setw(4) << data;
			}

		private:
			std::unique_ptr<Bundle> m_bundle;
		};
	}


	App::App(const cfg::json& config)
	{
		m_impl = std::make_unique<detail::AppImpl>(config);
	}

	App::~App()
	{}

	void App::mainloop()
	{
		m_impl->mainloop();
	}
}
