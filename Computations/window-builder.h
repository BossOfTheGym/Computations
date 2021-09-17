#pragma once

#include <cfg-fwd.h>

#include "main-window.h"

#include <memory>

using MainWindowPtr = std::unique_ptr<win::MainWindow>;

class WindowBuilder
{
public:
	/* json
		"window" : {
			"title" : <window_titile>,
			"width" : <window_width>,
			"height" : <window_height>
		}
	*/
	MainWindowPtr build(const cfg::json& config);
};
