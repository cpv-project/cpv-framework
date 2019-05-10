#pragma once
#include <typeindex>
#include "../Exceptions/ContainerException.hpp"
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
		
		/** Get service instance, throws exception if not registered or registered multiple times */
		// TODO
		
		/** Get service instances and adding them to given collection, notice it will not clear the collection first */
		// TODO
		
		/** Constructor */
		Container();
		
	private:
		/** Associate a descriptor to given service type */
		void addDescriptor(const std::type_index& serviceType, ServiceDescriptorPtr&& serviceDescriptor);
		
		/** Get all descriptors associated to given service type, return null pointer if not registered */
		const ServiceDescriptorCollection& getDescriptors(const std::type_index& serviceType) const&;
		
		/** Get all descriptors associated to given service type, return empty list if not registered */
		ServiceDescriptorCollection& getOrCreateEmptyDescriptors(const std::type_index& serviceType) &;
		
		/** Get service instance, throws exception if not registered or registered multiple times */
		template <class T>
		T get(const ServiceDescriptorCollection& descriptors, ServiceStorage& storage) {
			if (descriptors.get() == nullptr || descriptors->empty()) {
				throw ContainerException(CPV_CODEINFO,
					"get instance of type [", typeid(T).name(), "] failed: not registered");
			} else if (descriptors->size() > 0) {
				throw ContainerException(CPV_CODEINFO,
					"get instance of type [", typeid(T).name(), "] failed: registered multiple times");
			}
			return ServiceDescriptor<T>::cast(descriptors->front()).getInstance(*this, storage);
		}
		
		/** Get service instances and adding them to given collection, notice it will not clear the collection first */
		template <class T, std::enable_if_t<ServiceCollectionTrait<T>::IsCollection, int> = 0>
		void getMany(const ServiceDescriptorCollection& descriptors, ServiceStorage& storage, T& collection) {
			if (descriptors.get() == nullptr) {
				return;
			}
			using ElementType = typename ServiceDependencyTrait<T>::ElementType;
			for (auto& descriptor : *descriptors) {
				ServiceCollectionTrait<T>::add(collection,
					ServiceDescriptor<ElementType>::cast(descriptor).getInstance(*this, storage));
			}
		}
		
	private:
		seastar::shared_ptr<ContainerData> data_;
	};
}

