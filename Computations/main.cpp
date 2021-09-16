#include "app.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <exception>

#include "static-storage.h"

struct penis
{};

struct A
{};

REGISTER_RESOURCE(penis, A, abc);

struct B
{};



int main()
{
	for (auto& [name, resource] : ACCESS_STORAGE(penis, A)) {
		std::cout << name << std::endl;
	}

	/*std::unique_ptr<app::App> app;
	
	try {
		app = std::make_unique<app::App>();
		app->mainloop();
	}
	catch (std::runtime_error e) {
		std::cerr << "Runtime error : " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Unexpected error." << std::endl;
		std::terminate();
	}*/

	return 0;
}