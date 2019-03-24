#pragma once
#include <tuple>
#include <type_traits>
#include "./ServiceTraits.hpp"

namespace cpv {
	class Container;
	class ServiceStorage;
	
	/** Base class of service factory */
	template <class T>
	class ServiceFactoryBase {
	public:
		/**
		 * Create an instance of service.
		 * The container and storage arguments are for resolving dependencies,
		 * the lifetime of instance should not be managed here.
		 */
		virtual T operator()(const Container& container, ServiceStorage& storage) const = 0;
		
		/** Virtual destructor */
		virtual ~ServiceFactoryBase() = default;
	};
	
	/** Get instances of service by construct given implementation type with dependency injection */
	template <class TImplementation, class TService,
		class = std::enable_if_t<std::is_base_of_v<TService, TImplementation>>>
	class ServiceDependencyInjectionFactory : public ServiceFactoryBase<TService> {
	public:
		using DependencyTrait = ServiceDependencyTrait<TImplementation>;
		
		/** Create an instance of service */
		virtual TService operator()(const Container& container, ServiceStorage& storage) const {
			// TODO
			throw 1;
		}
		
		ServiceDependencyInjectionFactory(Container& container) : dependencyFactories_() {
			// TODO
		}
		
	private:
		// TODO
		std::array<int, std::tuple_size_v<DependencyTrait::DependencyTypes>> dependencyFactories_;
	};
}

