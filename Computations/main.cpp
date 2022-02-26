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
	for (auto split : {127, 255, 511, 1023}) {
		for (auto step : {3, 4, 5}) {
			for (auto work : {8, 16, 32}) {
				std::ostringstream output;
				output << "tests/tiled/test_" << split << "_" << step << "_" << work << ".json";

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
	builder.setWindowWidth(512);
	builder.setWindowHeight(512);
	builder.setTotalUpdates(1000);
	for (auto split : {127, 255, 511, 1023}) {
		for (auto work : {8, 16, 32}) {
			std::ostringstream output;
			output << "tests/simple/test_" << split << "_" << work<< ".json";

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
	builder.setSystems({"jacoby", "chaotic_tiled", "chaotic_smtm"});
	builder.setOutput("test/output.json");
	builder.setSplitX(512);
	builder.setSplitY(512);
	builder.setGridX(3);
	builder.setGridY(1);
	builder.setWindowWidth(1536);
	builder.setWindowHeight(512);
	builder.setTotalUpdates(100000);
	builder.setSteps(5);
	builder.setWorkgroupSizeX(8);
	builder.setWorkgroupSizeY(8);

	auto application = std::make_unique<app::App>(builder.build());
	application->mainloop();
}

int main()
{
	//test_tiled();
	//test_simple();	
	custom_test();
	return 0;
}
