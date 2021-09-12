#pragma once

#include <vector>
#include <string>
#include <utility>

class Dependency
{
public:
	using Storage = std::vector<std::string>;

	template<class storage_t>
	Dependency(storage_t&& allDependencies)
		: m_dependencies(std::forward<storage_t>(allDependencies))
	{}

	// NOTE : virtual is unneccessary here
	virtual ~Dependency() = default;

	const Storage& dependencies() const
	{
		return m_dependencies;
	}

private:
	Storage m_dependencies;
};
