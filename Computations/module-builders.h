#pragma once

#include <memory>
#include <string>
#include <cassert>
#include <utility>
#include <iostream>
#include <unordered_map>

#include <cfg-fwd.h>

#include "module.h"


class IModuleBuilder
{
public:
	virtual ~IModuleBuilder() = default;

	virtual std::shared_ptr<Module> build(Module& root, const cfg::json& config) = 0;
};


class ModuleBuilders
{
	template<class T>
	friend struct ModuleBuilderRegistrar;

	inline static std::unique_ptr<ModuleBuilders> m_instance;

public:
	static ModuleBuilders& instance()
	{
		if (!m_instance) {
			m_instance = std::make_unique<ModuleBuilders>();
		}
		return *m_instance;
	}

public:
	using Entry = std::unique_ptr<IModuleBuilder>;
	using Storage = std::unordered_map<std::string, Entry>;
	using Iterator = typename Storage::iterator;

	Iterator begin()
	{
		return m_storage.begin();
	}

	Iterator end()
	{
		return m_storage.end();
	}

	template<class T, class ... Args>
	bool reg(const std::string& name, Args&& ... args)
	{
		Entry builder = std::make_unique<T>(std::forward<Args>(args)...);
		auto [it, inserted] = m_storage.insert({name, std::move(builder)});
		return inserted;
	}

private:
	Storage m_storage;
};

#define REGISTER_MODULE_BUILDER(type, name, ...) \
inline bool dummy##type = ModuleBuilders::instance().reg<type>(name, ##__VA_ARGS__);
