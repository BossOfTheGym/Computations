#pragma once

#include <unordered_map>

#include <core.h>

#include "dependency.h"

class DependencyResolver
{
public:
	enum class Label
	{
		White,
		Gray,
		Black
	};

	struct Entry
	{
		Label label{};
		Dependencies dependencies;
	};

	using Storage  = std::unordered_map<std::string, Entry>;
	using Iterator = typename Storage::iterator;

public:
	void clear();

	bool addDependencies(const std::string& name, const Dependencies& dependencies);
	bool removeDependencies(const std::string& name);

	bool resolve(Dependencies& deps);

private:
	bool dfs(Iterator it);

private:
	Storage      m_storage;
	Dependencies m_order;
	uint m_curr{};
};
