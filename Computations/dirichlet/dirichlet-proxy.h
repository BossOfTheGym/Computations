#pragma once

#include <type_traits>

#include <dirichlet/dirichlet_handle.h>
#include <dirichlet/dirichlet_function.h>
#include <dirichlet/dirichlet_dataaabb2d.h>
#include <dirichlet/dirichlet_domainaabb2d.h>

namespace dir2d
{
	// TODO : check if T is dirichlet system, create utility template

	class Proxy
	{
	public:
		using CreateFunc = Handle(*)(void*, const DomainAabb2D&, const DataAabb2D&);
		using CreateSmartFunc = SmartHandle(*)(void*, const DomainAabb2D&, const DataAabb2D&);
		using DestroyFunc = void(*)(void*, Handle);
		using UpdateFunc = void(*)(void*);

		template<class T>
		Proxy(T& instance)
		{
			store(instance);
		}

		template<class T>
		void store(T& instance)
		{
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
			return m_destroyFunc(m_indtance, handle);
		}

		void update()
		{
			return m_updateFunc(m_instance);
		}

	private:
		void* m_instance{nullptr};
		CreateFunc      m_createFunc{nullptr};
		CreateSmartFunc m_createSmartFunc{nullptr};
		DestroyFunc     m_destroyFunc{nullptr};
		UpdateFunc      m_updateFunc{nullptr};
	};
}
