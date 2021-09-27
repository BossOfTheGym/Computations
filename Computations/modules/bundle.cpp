#include "bundle.h"

#include "module-builders.h"
#include "module-dependencies.h"

#include <dependency-resolver.h>

#include <stdexcept>

Bundle::Bundle(const cfg::json& config)
{
	DependencyResolver resolver;
	for (auto& [name, deps] : ACCESS_MODULE_BUILD_DEPENDENCIES()) {
		resolver.addDependencies(name, deps);
	}
	if (!resolver.resolve(m_constructionOrder)) {
		throw std::runtime_error("Failed to resolve construction order.");
	}

	auto& builders = ACCESS_MODULE_BUILDERS();
	for (auto& name : m_constructionOrder) {
		if (auto it = builders.find(name); it != builders.end()) {
			it->second->build(m_root, config);
		}
	}
}

Bundle::~Bundle()
{
	std::reverse(m_constructionOrder.begin(), m_constructionOrder.end());
	for (auto& name : m_constructionOrder) {
		if (!m_root.unload(name)) {
			// TODO : report error
		}
	}
}