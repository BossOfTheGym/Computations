#pragma once

#include <cfg-fwd.h>

#include "main-window.h"

class WindowBuilder
{
public:
	win::MainWindow build(const cfg::json& config);
};
