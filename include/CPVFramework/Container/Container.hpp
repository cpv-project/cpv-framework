#pragma once
#include <typeindex>
#include "./ServiceDescriptor.hpp"
#include "./ServiceFactory.hpp"

namespace cpv {
	/** Members of Container */
	class ContainerData;
	
	/**
	 * Dependency injection container.
	 * It can't use across cpu cores, please create one container for one cpu core.
	 */
	class Container {
	public:
		/** Register service with implementation type */
		template <class TService, class TImplementation>
		void add(ServiceLifetime lifetime = ServiceLifetime::Transient) {
			addDescriptor(typeid(TService), ServiceDescriptor<TService>::create(
				std::optional<TService>(),
				std::make_unique<ServiceDependencyInjectionFactory<TService, TImplementation>>(*this),
				lifetime));
		}
		
		/** Register service with instance */
		template <class TService>
		void add(TService instance) {
			addDescriptor(typeid(TService), ServiceDescriptor<TService>::create(
				std::move(instance),
				std::unique_ptr<ServiceFactoryBase<TService>>(), // TODO: replace with factory throws exception
				ServiceLifetime::Presistent));
		}
		
		/** Register service with custom factory function */
		// TODO
		
		/** Register service with custom factory object */
		// TODO
		
		/** Get service instance, throws exception if associated with none or multiple descriptors */
		// TODO
		
		/** Get service instances and adding them to given collection, notice it will not clear the collection first */
		// TODO: use ServiceCollectionTrait::add to add instances
		
		/** Constructor */
		Container();
		
	private:
		/** Associate a descriptor to given service type */
		void addDescriptor(const std::type_index& serviceType, ServiceDescriptorPtr&& serviceDescriptor);
		
		/** Get all descriptors associated to given service type, return null pointer if not registered */
		const ServiceDescriptorCollection& getDescriptors(const std::type_index& serviceType) const&;
		
		/** Get all descriptors associated to given service type, return empty list if not registered */
		ServiceDescriptorCollection& getOrCreateEmptyDescriptors(const std::type_index& serviceType) &;
		
	private:
		seastar::shared_ptr<ContainerData> data_;
	};
}

