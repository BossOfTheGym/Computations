#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <placeholder.h>

class Module : public Placeholder
{
public:
	using Placeholder::Placeholder;

	using Entry = std::shared_ptr<Module>;
	using Storage = std::unordered_map<std::string, Entry>;
	using Iterator = typename Storage::iterator;
	using ConstIterator = typename Storage::const_iterator;
	using Status = std::pair<Iterator, bool>;

	Entry acquire(const std::string& name);

	Status load(const std::string& name, Entry entry);
	bool unload(const std::string& name);

	Iterator find(const std::string& name);
	ConstIterator find(const std::string& name) const;
	bool has(const std::string& name) const;

	Iterator begin();
	Iterator end();
	ConstIterator begin() const;
	ConstIterator end() const;

private:
	Storage m_modules;
};

using ModulePtr = std::shared_ptr<Module>;
