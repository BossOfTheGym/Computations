#pragma once

#include "type-id.h"

#include <cassert>
#include <memory>
#include <utility>
#include <type_traits>

namespace detail
{
	struct IPlaceholder
	{
		virtual ~IPlaceholder() = default;
	};

	template<class T>
	struct Placeholder : public IPlaceholder
	{
		Placeholder(const T& value) : object(value)
		{
			static_assert(std::is_copy_constructible_v<T>, "Placeholder : type is not copy constructible.");
		}

		Placeholder(T&& value) : object(std::move(value))
		{
			static_assert(std::is_move_constructible_v<T>, "Placeholder : type is not move constructible.");
		}

		template<class ... Args, class TT = T, std::enable_if_t<std::is_aggregate_v<TT>, int> = 0>
		Placeholder(Args&& ... args) : object{std::forward<Args>(args)...}
		{}

		template<class ... Args, class TT = T, std::enable_if_t<!std::is_aggregate_v<TT>, int> = 0>
		Placeholder(Args&& ... args) : object(std::forward<Args>(args)...)
		{}

		T object;
	};
}

template<class type>
struct placeholder_type {};

class Placeholder
{
public:
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
