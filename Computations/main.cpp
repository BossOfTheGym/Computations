#include "app.h"

#include <modules/bundle.h>
#include <config-builder.h>

#include <memory>
#include <sstream>

void test_tiled()
{
	ConfigBuilder builder;
	builder.setSystems({"red_black_tiled", "red_black_smtm", "red_black_smtm_s"});
	builder.setGridX(3);
	builder.setGridY(1);
	builder.setWindowWidth(1536);
	builder.setWindowHeight(512);
	builder.setTotalUpdates(1000);
	for (auto split : {63, 127, 255, 511, 1023, 2047}) {
		for (auto step : {3, 4, 5, 6}) {
			for (auto work : {8, 16, 32}) {
				std::ostringstream output;
				output << "tests/tiled_" << split << "_" << step << "_" << work << ".json";

				builder.setSteps(step);
				builder.setSplitX(split);
				builder.setSplitY(split);
				builder.setWorkgroupSizeX(work);
				builder.setWorkgroupSizeY(work);
				builder.setOutput(output.str());

				auto application = std::make_unique<app::App>(builder.build());
				application->mainloop();
			}
		}
	}
}

void test_simple()
{
	ConfigBuilder builder;
	builder.setSystems({"red_black"});
	builder.setGridX(1);
	builder.setGridY(1);
	builder.setWindowWidth(1024);
	builder.setWindowHeight(1024);
	builder.setTotalUpdates(1000);
	for (auto split : {63, 127, 255, 511, 1023, 2047}) {
		for (auto work : {8, 16, 32}) {
			std::ostringstream output;
			output << "tests/simple_" << split << "_" << work<< ".json";

			builder.setSplitX(split);
			builder.setSplitY(split);
			builder.setWorkgroupSizeX(work);
			builder.setWorkgroupSizeY(work);
			builder.setOutput(output.str());

			auto application = std::make_unique<app::App>(builder.build());
			application->mainloop();
		}
	}
}

void custom_test()
{
	ConfigBuilder builder;
	builder.setSystems({"red_black_tiled", "red_black_smtm"});
	builder.setOutput("test/output.json");
	builder.setSplitX(2047);
	builder.setSplitY(2047);
	builder.setGridX(2);
	builder.setGridY(1);
	builder.setWindowWidth(1024);
	builder.setWindowHeight(512);
	builder.setTotalUpdates(1000);
	builder.setSteps(5);
	builder.setWorkgroupSizeX(32);
	builder.setWorkgroupSizeY(32);

	auto application = std::make_unique<app::App>(builder.build());
	application->mainloop();
}

int main()
{
	test_tiled();
	test_simple();	

	return 0;
}
