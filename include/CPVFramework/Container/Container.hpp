#pragma once
#include <typeindex>
#include "./ServiceDescriptor.hpp"
#include "./ServiceFactory.hpp"
#include "./ServiceStorage.hpp"

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
				std::make_unique<ServiceExceptionFactory<TService>>(
					"it's registered with instance but still try to invoke factory"),
				ServiceLifetime::Presistent));
		}
		
		/** Register service with function that returns instance of service */
		template <class TService, class TFunc,
			std::enable_if_t<std::is_base_of_v<
				ServiceFactoryBase<TService>,
				ServiceFunctionFactory<TService, TFunc>>, int> = 0>
		void add(TFunc&& func, ServiceLifetime lifetime = ServiceLifetime::Transient) {
			addDescriptor(typeid(TService), ServiceDescriptor<TService>::create(
				std::optional<TService>(),
				std::make_unique<ServiceFunctionFactory<TService, TFunc>>(
					std::is_lvalue_reference_v<TFunc> ? TFunc(func) : std::move(func)),
				lifetime));
		}
		
		/** Register service with custom factory object */
		template <class TService>
		void add(std::unique_ptr<ServiceFactoryBase<TService>>&& factory,
			ServiceLifetime lifetime = ServiceLifetime::Transient) {
			addDescriptor(typeid(TService), ServiceDescriptor<TService>::create(
				std::optional<TService>(), std::move(factory), lifetime));
		}
		
		/** Get service instance, throws exception if not registered or registered multiple times */
		template <class T>
		T get() const {
			// use built-in service storage
			return get<T>(getBuiltinStorage());
		}
		
		/** Get service instance, throws exception if not registered or registered multiple times */
		template <class T>
		T get(ServiceStorage& storage) const {
			return get<T>(getDescriptors(typeid(T)), storage);
		}
		
		/** Get service instances and adding them to given collection, it will not clear items first */
		template <class T, std::enable_if_t<ServiceCollectionTrait<T>::IsCollection, int> = 0>
		std::size_t getMany(T& collection) const {
			return getMany<T>(getBuiltinStorage(), collection);
		}
		
		/** Get service instances and adding them to given collection, it will not clear items first */
		template <class T, std::enable_if_t<ServiceCollectionTrait<T>::IsCollection, int> = 0>
		std::size_t getMany(ServiceStorage& storage, T& collection) const {
			return getMany<T>(getDescriptors(typeid(T)), storage, collection);
		}
		
		/** Constructor */
		Container();
		
	private:
		/** Associate a descriptor to given service type */
		void addDescriptor(const std::type_index& serviceType, ServiceDescriptorPtr&& serviceDescriptor);
		
		/** Get all descriptors associated to given service type, return null pointer if not registered */
		const ServiceDescriptorCollection& getDescriptors(const std::type_index& serviceType) const&;
		
		/** Get all descriptors associated to given service type, return empty list if not registered */
		ServiceDescriptorCollection& getOrCreateEmptyDescriptors(const std::type_index& serviceType) &;
		
		/** Get built-in service storage */
		ServiceStorage& getBuiltinStorage() const&;
		
		/** Get service instance, throws exception if not registered or registered multiple times */
		template <class T>
		T get(const ServiceDescriptorCollection& descriptors, ServiceStorage& storage) const {
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
		std::size_t getMany(
			const ServiceDescriptorCollection& descriptors, ServiceStorage& storage, T& collection) const {
			if (descriptors.get() == nullptr) {
				return 0;
			}
			using ElementType = typename ServiceDependencyTrait<T>::ElementType;
			for (auto& descriptor : *descriptors) {
				ServiceCollectionTrait<T>::add(collection,
					ServiceDescriptor<ElementType>::cast(descriptor).getInstance(*this, storage));
			}
			return descriptors->size();
		}
		
	private:
		seastar::shared_ptr<ContainerData> data_;
	};
}

