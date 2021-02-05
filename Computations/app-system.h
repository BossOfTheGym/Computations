#pragma once


namespace app
{
	class App;

	// All systems, must have reference to App
	class System
	{
	public:
		System(App& mainApp) : m_app{&mainApp}
		{}

		System(const System&) = delete;
		System(System&&) = delete;

		System& operator = (const System&) = delete;
		System& operator = (System&&) = delete;

		virtual ~System() = default;

	public:
		App* app() const
		{
			return m_app;
		}

	private:
		App* m_app;
	};
}