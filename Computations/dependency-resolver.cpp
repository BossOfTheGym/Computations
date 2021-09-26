#include "dependency-resolver.h"

void DependencyResolver::clear()
{
	m_storage.clear();
	m_order.clear();
}

bool DependencyResolver::addDependencies(const std::string& name, const Dependencies& dependencies)
{
	auto [it, inserted] = m_storage.insert({name, {Label::White, dependencies}});
	return inserted;
}

bool DependencyResolver::removeDependencies(const std::string& name)
{
	return m_storage.erase(name) != 0;
}

bool DependencyResolver::resolve(Dependencies& deps)
{
	m_order.resize(m_storage.size());

	m_curr = 0;
	for (auto it = m_storage.begin(), e = m_storage.end(); it != e; it++) {
		auto& [name, entry] = *it;
		if (entry.label == Label::White && !dfs(it)) {
			return false;
		}
	}

	deps = std::move(m_order);
	m_storage.clear();

	return true;
}

bool DependencyResolver::dfs(Iterator it)
{
	if (it == m_storage.end()) { // dangling vertex found, error
		return false;
	}
	
	auto& [name, entry] = *it;
	if (entry.label == Label::Gray) { // cycle detected, cannot build dependency order
		return false;
	}
	if (entry.label == Label::Black) { // already visited, return success
		return true;
	}

	entry.label = Label::Gray;
	for (auto& dep : entry.dependencies) {
		auto next = m_storage.find(dep);
		if (!dfs(next)) {
			return false;
		}
	}
	entry.label = Label::Black;

	m_order[m_curr++] = name;

	return true;
}
