#include "app.h"

#include <modules/bundle.h>
#include <config-builder.h>

#include <memory>

int main()
{
	ConfigBuilder builder;
	builder.setSystems({"red_black_tiled", "red_black_smt"});
	builder.setOutput("test");
	builder.setSplitX(1024);
	builder.setSplitY(1024);
	builder.setGridX(2);
	builder.setGridY(1);
	builder.setWindowWidth(1024);
	builder.setWindowHeight(512);
	builder.setTotalUpdates(10000000);
	builder.setWorkgroupSizeX(16);
	builder.setWorkgroupSizeY(16);

	auto application = std::make_unique<app::App>(builder.build());
	application->mainloop();

	return 0;
}