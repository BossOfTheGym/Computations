#pragma once

#include <cfg.h>
#include <core.h>

#include <string>

// utility to generate several versions configurations
class ConfigGenerator
{
public:
	cfg::json generate(
		std::string windowName,
		int windowWidth,
		int windowHeight, 
		uint workgroupX, 
		uint workgroupY, 
		uint steps);

private:
	cfg::json m_config;
};