#pragma once

#include "module.h"

#include <cfg-fwd.h>
#include <dependency.h>

class Bundle
{
public:
	Bundle(const cfg::json& config);
	~Bundle();

	Module& root()
	{
		return m_root;
	}

private:
	Module m_root;
	Dependencies m_constructionOrder;
};