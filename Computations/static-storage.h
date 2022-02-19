#pragma once

#include <string>
#include <utility>
#include <unordered_map>

template<class Tag>
struct StaticStorage
{
	template<class stored_t>
	struct Storage
	{
		std::unordered_map<std::string, stored_t> storage;

		bool store(const std::string& name, const stored_t& item)
		{
			auto [_, inserted] = storage.insert({name, item});
			return inserted;
		}

		bool store(const std::string& name, stored_t&& item)
		{
			auto [_, inserted] = storage.insert({name, std::move(item)});
			return inserted;
		}

		template<class ... Args>
		bool store(const std::string& name, Args&& ... args)
		{
			if constexpr (std::is_aggregate_v<stored_t>) {
				auto [_, inserted] = storage.insert({name, stored_t{std::forward<Args>(args)...}});
				return inserted;
			}
			else {
				auto [_, inserted] = storage.insert({name, stored_t(std::forward<Args>(args)...)});
				return inserted;
			}
		}
	};

	template<class stored_t>
	static Storage<stored_t>& instance()
	{
		if (!m_instance<stored_t>) {
			m_instance<stored_t> = std::make_unique<Storage<stored_t>>();
		}
		return *m_instance<stored_t>;
	}

private:
	template<class stored_t>
	inline static std::unique_ptr<Storage<stored_t>> m_instance;
};

#define ACCESS_STORAGE(tag, resource) StaticStorage<tag>::instance<resource>().storage

#define REGISTER_RESOURCE(tag, resource, name, ...) \
inline bool staticStorage_##tag##name = StaticStorage<tag>::instance<resource>().store(#name, ##__VA_ARGS__)
