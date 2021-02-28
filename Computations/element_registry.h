#pragma once

#include <entt/core/family.hpp>

#include <cassert>
#include <vector>
#include <memory>

namespace app
{
	template<class Tag>
	class ElementRegistry
	{
	private:
		struct IPlaceholder
		{
			virtual ~IPlaceholder() = default;
		};

		template<class T>
		struct Placeholder : IPlaceholder
		{
			template<class ... Args, class TT = T, std::enable_if_t<std::is_aggregate_v<TT>, int> = 0>
			Placeholder(Args&& ... args) : object{std::forward<Args>(args)...}
			{}

			template<class ... Args, class TT = T, std::enable_if_t<!std::is_aggregate_v<TT>, int> = 0>
			Placeholder(Args&& ... args) : object(std::forward<Args>(args)...)
			{}

			T object;
		};

		using Id = typename entt::family<Tag>::family_type;

	public:
		ElementRegistry() = default;

		ElementRegistry(const ElementRegistry&) = delete;
		ElementRegistry(ElementRegistry&&)      = delete;

		~ElementRegistry() = default;

		ElementRegistry& operator = (const ElementRegistry&) = delete;
		ElementRegistry& operator = (ElementRegistry&&)      = delete;

	private:
		template<class Element>
		Id elementId()
		{
			return entt::family<Tag>::template type<Element>;
		}

		void assure(Id id)
		{
			if (id >= m_elements.size())
			{
				m_elements.resize(id + 1);
			}
		}

	public:
		template<class Element, class ... Args>
		void add(Args&& ... args)
		{
			assert(!has<Element>());

			auto id = elementId<Element>();

			assure(id);
			m_elements[id] = std::make_unique<Placeholder<Element>>(std::forward<Args>(args)...);
		}

		template<class Element>
		void remove()
		{
			assert(has<Element>());

			auto id = elementId<Element>();

			m_elements[id].reset();
		}

		template<class Element>
		bool has()
		{
			auto id = elementId<Element>();

			return id < m_elements.size() && m_elements[id] != nullptr;
		}

		template<class Element>
		Element& get()
		{
			assert(has<Element>());

			auto id = elementId<Element>();

			return static_cast<Placeholder<Element>&>(*m_elements[id]).object;
		}

	private:
		std::vector<std::unique_ptr<IPlaceholder>> m_elements;
	};
};
