#pragma once

#include <string>
#include <memory>
#include <unordered_map>

class Module
{
public:
	using Entry =  std::shared_ptr<Module>;
	using Storage = std::unordered_map<std::string, Entry>;
	using Iterator = typename Storage::iterator;
	using ConstIterator = typename Storage::const_iterator;
	using Status = std::pair<Iterator, bool>;

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
