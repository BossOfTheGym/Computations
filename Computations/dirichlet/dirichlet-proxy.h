#pragma once

#include <type_traits>

#include <gl-cxx/gl-types.h>
#include <dirichlet/dirichlet_handle.h>
#include <dirichlet/dirichlet_function.h>
#include <dirichlet/dirichlet_dataaabb2d.h>
#include <dirichlet/dirichlet_domainaabb2d.h>

namespace dir2d
{
	template<class T, class = void>
	struct is_dirichlet_system : std::false_type 
	{};

	template<class T>
	struct is_dirichlet_system<T, 
		std::enable_if_t<
			std::is_invocable_r_v<Handle, decltype(&T::create), T*, const DomainAabb2D&, const DataAabb2D&>
			&& std::is_invocable_r_v<SmartHandle, decltype(&T::createSmart), T*, const DomainAabb2D&, const DataAabb2D&>
			&& std::is_invocable_r_v<void, decltype(&T::destroy), T*>
			&& std::is_invocable_r_v<void, decltype(&T::update), T*>
			&& std::is_invocable_r_v<GLuint64, decltype(&T::elapsed), T*>
			&& std::is_invocable_r_v<f64, decltype(&T::elapsedMean), T*>
		>
	> : std::true_type
	{};

	template<class T>
	constexpr bool is_dirichlet_system_v = is_dirichlet_system<T>::value;

	class Proxy
	{
	public:
		using CreateFunc = Handle(*)(void*, const DomainAabb2D&, const DataAabb2D&);
		using CreateSmartFunc = SmartHandle(*)(void*, const DomainAabb2D&, const DataAabb2D&);
		using DestroyFunc = void(*)(void*, Handle);
		using UpdateFunc = void(*)(void*);
		using ElapsedFunc = GLuint64(*)(void*);
		using ElapsedMeanFunc = f64(*)(void*);

		template<class T>
		Proxy(T& instance)
		{
			store(instance);
		}

		template<class T>
		void store(T& instance)
		{
			static_assert(is_dirichlet_system_v<T>, "T is not a dirichlet system");

			m_instance = &instance;

			m_createFunc = [] (void* inst, const DomainAabb2D& domain, const DataAabb2D& data)
			{
				return static_cast<T*>(inst)->create(domain, data);
			};
			m_createSmartFunc = [] (void* inst, const DomainAabb2D& domain, const DataAabb2D& data)
			{
				return static_cast<T*>(inst)->createSmart(domain, data);
			};
			m_destroyFunc = [] (void* inst, Handle handle)
			{
				return static_cast<T*>(inst)->destroy(handle);
			};
			m_updateFunc = [] (void* inst)
			{
				return static_cast<T*>(inst)->update();
			};
			m_elapsedFunc = [] (void* inst)
			{
				return static_cast<T*>(inst)->elapsed();
			};
			m_elapsedMeanFunc = [] (void* inst)
			{
				return static_cast<T*>(inst)->elapsedMean();
			};
		}

		Handle create(const DomainAabb2D& domain, const DataAabb2D& data)
		{
			return m_createFunc(m_instance, domain, data);
		}

		SmartHandle createSmart(const DomainAabb2D& domain, const DataAabb2D& data)
		{
			return m_createSmartFunc(m_instance, domain, data);
		}

		void destroy(Handle handle)
		{
			return m_destroyFunc(m_instance, handle);
		}

		void update()
		{
			return m_updateFunc(m_instance);
		}

		GLuint64 elapsed()
		{
			return m_elapsedFunc(m_instance);
		}

		f64 elapsedMean()
		{
			return m_elapsedMeanFunc(m_instance);
		}

	private:
		void* m_instance{nullptr};
		CreateFunc      m_createFunc{nullptr};
		CreateSmartFunc m_createSmartFunc{nullptr};
		DestroyFunc     m_destroyFunc{nullptr};
		UpdateFunc      m_updateFunc{nullptr};
		ElapsedFunc     m_elapsedFunc{nullptr};
		ElapsedMeanFunc m_elapsedMeanFunc{nullptr};
	};
}
