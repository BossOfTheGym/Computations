#include "app.h"

#include <modules/bundle.h>
#include <config-builder.h>

#include <memory>
#include <sstream>

void test_tiled(const std::vector<std::string>& systems,
				uint width,
				const std::vector<uint>& splitValues,
				const std::vector<uint>& stepValues,
				const std::vector<uint>& workValues,
				const std::string& outputPrefix,
				uint updates)
{
	ConfigBuilder builder;
	builder.setSystems(systems);
	builder.setGridX(systems.size());
	builder.setGridY(1);
	builder.setWindowWidth(width * systems.size());
	builder.setWindowHeight(width);
	builder.setTotalUpdates(updates);
	for (auto split : splitValues) {
		for (auto step : stepValues) {
			for (auto work : workValues) {
				std::ostringstream output;
				output << outputPrefix << split << "_" << step << "_" << work << ".json";

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

void test_non_tiled(const std::vector<std::string>& systems,
				 uint width,
				 const std::vector<uint>& splitValues,
				 const std::vector<uint>& workValues,
				 const std::string& outputPrefix,
				 uint updates)
{
	ConfigBuilder builder;
	builder.setSystems(systems);
	builder.setGridX(1);
	builder.setGridY(1);
	builder.setWindowWidth(width);
	builder.setWindowHeight(width);
	builder.setTotalUpdates(updates);
	for (auto split : splitValues) {
		for (auto work : workValues) {
			std::ostringstream output;
			output << outputPrefix << split << "_" << work << ".json";

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

void test_rb_tiled()
{
	test_tiled({"red_black_tiled", "red_black_smtm", "red_black_smtm_s"},
			   512,
			   {255, 511, 1023},
			   {3, 4, 5},
			   {16, 24, 32},
			   "tests/tiled/test_",
			   1000);
}

void test_chaotic()
{
	test_tiled({"chaotic_tiled", "chaotic_smtm"},
			   512,
			   {255, 511, 1023},
			   {3, 4, 5},
			   {12, 16, 20},
			   "tests/chaotic/test_",
			   1000);
}

void test_rb()
{
	test_non_tiled({"red_black"},
				   512,
				   {255, 511, 1023},
				   {16, 24, 32},
				   "tests/rb/test_",
				   1000);
}

void test_jacoby()
{
	test_non_tiled({"jacoby"},
				   512,
				   {255, 511, 1023},
				   {12, 16, 20},
				   "tests/jacoby/test_",
				   1000);
}

void test_all()
{
	test_rb_tiled();
	test_chaotic();
	test_rb();
	test_jacoby();
}

void custom_test()
{
	ConfigBuilder builder;
	builder.setSystems({"chaotic_tiled", "chaotic_smtm", "jacoby"});
	builder.setOutput("test/output.json");
	builder.setSplitX(1023);
	builder.setSplitY(1023);
	builder.setGridX(3);
	builder.setGridY(1);
	builder.setWindowWidth(1536);
	builder.setWindowHeight(512);
	builder.setTotalUpdates(200);
	builder.setSteps(5);
	builder.setWorkgroupSizeX(16);
	builder.setWorkgroupSizeY(16);

	auto application = std::make_unique<app::App>(builder.build());
	application->mainloop();
}

int main()
{
	test_all();
	//custom_test();
	return 0;
}
