#include "module.h"

auto Module::load(const std::string& name, Entry entry) -> Status
{
	return m_modules.insert({name, std::move(entry)});
}

bool Module::unload(const std::string& name)
{
	return m_modules.erase(name) != 0;
}

auto Module::find(const std::string& name) -> Iterator
{
	return m_modules.find(name);
}

auto Module::find(const std::string& name) const -> ConstIterator
{
	return m_modules.find(name);
}

bool Module::has(const std::string& name) const
{
	return find(name) != end();
}

Module& Module::get(const std::string& name)
{
	return *find(name)->second;
}

const Module& Module::get(const std::string& name) const
{
	return *find(name)->second;
}

auto Module::begin() -> Iterator
{
	return m_modules.begin();
}

auto Module::end() -> Iterator
{
	return m_modules.end();
}

auto Module::begin() const -> ConstIterator
{
	return m_modules.begin();
}

auto Module::end() const -> ConstIterator
{
	return m_modules.end();
}
