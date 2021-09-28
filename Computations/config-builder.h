#pragma once

#include <cfg.h>
#include <core.h>

#include <string>
#include <vector>

class ConfigBuilder
{
public:
	// TODO : specify loaded systems
	// TODO : steps for tiled systems
	// TODO : total_updates

	cfg::json build();


	void setOutput(const std::string& value)
	{
		m_output = value;
	}

	void setSystems(const std::vector<std::string>& value)
	{
		m_systems = value;
	}


	void setGridX(uint value)
	{
		m_gridX = value;
	}

	void steGridY(uint value)
	{
		m_gridY = value;
	}


	void setWindowWidth(uint value)
	{
		m_windowWidth = value;
	}

	void setWindowHeight(uint value)
	{
		m_windowHeight = value;
	}


	void setSplitX(uint value)
	{	
		m_xSplit = value;
	}

	void setSplitY(uint value)
	{
		m_ySplit = value;
	}

	void setTotalUpdates(uint value)
	{
		m_totalUpdates = value;
	}

	void setWorkgroupSizeX(uint value)
	{
		m_workgroupSizeX = value;
	}

	void setWorkgroupSizeY(uint value)
	{
		m_workgroupSizeY = value;
	}

	void setSteps(uint value)
	{
		m_steps = value;
	}

private:
	std::string m_output;
	std::vector<std::string> m_systems;

	uint m_gridX{3};
	uint m_gridY{2};
	
	uint m_windowWidth{1200};
	uint m_windowHeight{800};

	uint m_xSplit{256};
	uint m_ySplit{256};
	uint m_totalUpdates{1000};
	uint m_workgroupSizeX{16};
	uint m_workgroupSizeY{16};
	uint m_steps{2};
};