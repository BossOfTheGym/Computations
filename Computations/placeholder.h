#pragma once

#include "type-id.h"

#include <cassert>
#include <memory>
#include <utility>
#include <type_traits>

template<class type>
struct placeholder_type
{};

template<class type>
inline constexpr placeholder_type<type> placeholder_t;

namespace detail
{
	struct IPlaceholder
	{
		virtual ~IPlaceholder() = default;
	};

	template<class T>
	struct Placeholder : public IPlaceholder
	{
		struct aggregate_tag
		{};

		struct non_aggregate_tag
		{};

		template<class ... Args>
		Placeholder(Args&& ... args) 
			: Placeholder(
				std::conditional_t<std::is_aggregate_v<T>, aggregate_tag, non_aggregate_tag>{},
				std::forward<Args>(args)...
			)
		{}

	private:
		template<class ... Args>
		Placeholder(aggregate_tag, Args&& ... args) : object{std::forward<Args>(args)...}
		{}

		template<class ... Args>
		Placeholder(non_aggregate_tag, Args&& ... args) : object(std::forward<Args>(args)...)
		{}

	public:
		T object;
	};
}

class Placeholder
{
public:
	Placeholder() = default;

	template<class T>
	Placeholder(const T& value)
	{
		store<T>(value);
	}

	template<class T>
	Placeholder(T&& value)
	{
		store<T>(std::move(value));
	}

	template<class T, class ... Args>
	Placeholder(placeholder_type<T>, Args&& ... args)
	{
		store<T>(std::forward<Args>(args)...);
	}

	template<class T>
	void store(const T& value)
	{
		m_placeholder = std::make_unique<detail::Placeholder<T>>(value);
		m_type = getTypeId<T>();
	}

	template<class T>
	void store(T&& value)
	{
		m_placeholder = std::make_unique<detail::Placeholder<T>>(std::move(value));
		m_type = getTypeId<T>();
	}

	template<class T, class ... Args>
	void store(Args&& ... args)
	{
		m_placeholder = std::make_unique<detail::Placeholder<T>>(std::forward<Args>(args)...);
		m_type = getTypeId<T>();
	}

public:
	template<class T>
	bool stores() const
	{
		return m_type == getTypeId<T>();
	}

	template<class T>
	T& get()
	{
		assert(stores<T>());

		return static_cast<detail::Placeholder<T>&>(*m_placeholder).object;
	}

private:
	template<class T>
	id_t getTypeId() const
	{
		return type_id<Placeholder>::template id<T>;
	}

private:
	std::unique_ptr<detail::IPlaceholder> m_placeholder;
	id_t m_type{null_id};
};

