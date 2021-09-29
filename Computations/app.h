#pragma once

#include <cfg-fwd.h>
#include <memory>

namespace app
{
	namespace detail
	{
		class AppImpl;	
	}

	class App
	{
	public:
		App(const cfg::json& config);

		App(const App&) = delete;
		App(App&&) = delete;

		App& operator = (const App&) = delete;
		App& operator = (App&&) = delete;

		~App();

	public:
		void mainloop();

	private:
		std::unique_ptr<detail::AppImpl> m_impl;
	};
}
