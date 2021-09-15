#pragma once

#include <cfg-fwd.h>

namespace app
{
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
	};
}
