#include "app.h"

int main()
{
	std::unique_ptr<app::App> app(new app::App());
	if (app->init())
	{
		app->mainloop();
	}
	else
	{
		std::cerr << "Failed to initialize application" << std::endl;

		return 1;
	}

	return 0;
}