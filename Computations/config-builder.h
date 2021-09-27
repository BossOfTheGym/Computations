#pragma once

#include <cfg.h>
#include <core.h>

#include <string>

class ConfigBuilder
{
public:
	cfg::json build();

	void setOutput(const std::string& value)
	{
		m_output = value;
	}

	void setSplitX(uint value)
	{	
		m_xSplit = value;
	}

	void setSplitY(uint value)
	{
		m_ySplit = value;
	}

	void setWorkgroupSizeX(uint value)
	{
		m_workgroupSizeX = value;
	}

	void setWorkgroupSizeY(uint value)
	{
		m_workgroupSizeY = value;
	}

private:
	std::string m_output;
	uint m_xSplit{};
	uint m_ySplit{};
	uint m_workgroupSizeX{};
	uint m_workgroupSizeY{};
};