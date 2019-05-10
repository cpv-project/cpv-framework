#pragma once
#include <tuple>
#include <type_traits>
#include <memory>
#include <seastar/core/shared_ptr.hh>
#include "../Utility/Object.hpp"
#include "./ServiceTraits.hpp"
#include "./ServiceDescriptorBase.hpp"
#include "./ServiceFactoryBase.hpp"

namespace cpv {
	/** Extensions functions for dependency types (tuple<...>) */
	template <class DependencyTypes, class ContainerType>
	struct DependencyTypesExtensions {
		static const constexpr std::size_t Size = std::tuple_size_v<DependencyTypes>;
		static const constexpr auto IndexSequence = std::make_index_sequence<Size>();
		using ArrayType = std::array<ServiceDescriptorCollection, Size>;
		template <std::size_t I>
		using DependencyType = std::tuple_element_t<I, DependencyTypes>;
		
		/** Get instance of single dependency type */
		template <class T, std::enable_if_t<!ServiceCollectionTrait<T>::IsCollection, int> = 0>
		static T getDependencyInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			const ServiceDescriptorCollection& descriptors) {
			return container.template get<T>(descriptors, storage);
		}
		
		/** Get instance of single dependency type, for collection */
		template <class T, std::enable_if_t<ServiceCollectionTrait<T>::IsCollection, int> = 0>
		static T getDependencyInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			const ServiceDescriptorCollection& descriptors) {
			T collection;
			container.template getMany<T>(descriptors, storage, collection);
			return collection;
		}
		
		/** Get array contains service descriptors for each dependency types */
		template <std::size_t... I>
		static ArrayType getDependencyDescriptorsImpl(
			ContainerType& container, std::index_sequence<I...>) {
			return ArrayType(container.getOrCreateEmptyDescriptors(typeid(DependencyType<I>))...);
		}
		
		/** Get array contains service descriptors for each dependency types */
		static ArrayType getDependencyDescriptors(Container& container) {
			return getDependencyDescriptorsImpl(container, IndexSequence);
		}
	};
	
	/** Get instances of service by construct given implementation type with dependency injection */
	template <class TService, class TImplementation,
		class = std::enable_if_t<std::is_base_of_v<TService, TImplementation>>>
	class ServiceDependencyInjectionFactory : public ServiceFactoryBase<TService> {
	public:
		using DependencyTrait = ServiceDependencyTrait<TImplementation>;
		using DependencyTypes = typename DependencyTrait::DependencyTypes;
		using Extensions = DependencyTypesExtensions<DependencyTypes, Container>;
		
		/** Create an instance of service */
		virtual TService operator()(const Container& container, ServiceStorage& storage) const {
			return getServiceInstance<Container>(container, storage, Extensions::IndexSequence);
		}
	
		/** Constructor **/
		ServiceDependencyInjectionFactory(Container& container) :
			dependencyDescriptors_(Extensions::getDependencyDescriptors(container)) { }
		
	private:
		/** Get instance of service */
		template <class ContainerType, std::size_t... I>
		TService getServiceInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			std::index_sequence<I...>) {
			return TService(Extensions::template
				getDependencyInstance<Extensions::DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** std::unique_ptr version of ServiceDependencyInjectionFactory */
	template <class TService, class TImplementation>
	class ServiceDependencyInjectionFactory<
		std::unique_ptr<TService>,
		std::unique_ptr<TImplementation>,
		std::enable_if_t<std::is_base_of_v<TService, TImplementation>>> :
		public ServiceFactoryBase<std::unique_ptr<TService>> {
	public:
		using DependencyTrait = ServiceDependencyTrait<TImplementation>;
		using DependencyTypes = typename DependencyTrait::DependencyTypes;
		using Extensions = DependencyTypesExtensions<DependencyTypes, Container>;
		
		/** Create an instance of service */
		virtual std::unique_ptr<TService> operator()(
			const Container& container, ServiceStorage& storage) const {
			return getServiceInstance<Container>(container, storage, Extensions::IndexSequence);
		}
	
		/** Constructor **/
		ServiceDependencyInjectionFactory(Container& container) :
			dependencyDescriptors_(Extensions::getDependencyDescriptors(container)) { }
		
	private:
		/** Get instance of service */
		template <class ContainerType, std::size_t... I>
		std::unique_ptr<TService> getServiceInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			std::index_sequence<I...>) {
			return std::make_unique<TService>(Extensions::template
				getDependencyInstance<Extensions::DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** seastar::shared_ptr version of ServiceDependencyInjectionFactory */
	template <class TService, class TImplementation>
	class ServiceDependencyInjectionFactory<
		seastar::shared_ptr<TService>,
		seastar::shared_ptr<TImplementation>,
		std::enable_if_t<std::is_base_of_v<TService, TImplementation>>> :
		public ServiceFactoryBase<seastar::shared_ptr<TService>> {
	public:
		using DependencyTrait = ServiceDependencyTrait<TImplementation>;
		using DependencyTypes = typename DependencyTrait::DependencyTypes;
		using Extensions = DependencyTypesExtensions<DependencyTypes, Container>;
		
		/** Create an instance of service */
		virtual seastar::shared_ptr<TService> operator()(
			const Container& container, ServiceStorage& storage) const {
			return getServiceInstance<Container>(container, storage, Extensions::IndexSequence);
		}
	
		/** Constructor **/
		ServiceDependencyInjectionFactory(Container& container) :
			dependencyDescriptors_(Extensions::getDependencyDescriptors(container)) { }
		
	private:
		/** Get instance of service */
		template <class ContainerType, std::size_t... I>
		seastar::shared_ptr<TService> getServiceInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			std::index_sequence<I...>) {
			return seastar::make_shared<TService>(Extensions::template
				getDependencyInstance<Extensions::DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** Object version of ServiceDependencyInjectionFactory */
	template <class TService, class TImplementation>
	class ServiceDependencyInjectionFactory<
		Object<TService>,
		Object<TImplementation>,
		std::enable_if_t<std::is_base_of_v<TService, TImplementation>>> :
		public ServiceFactoryBase<Object<TService>> {
	public:
		using DependencyTrait = ServiceDependencyTrait<TImplementation>;
		using DependencyTypes = typename DependencyTrait::DependencyTypes;
		using Extensions = DependencyTypesExtensions<DependencyTypes, Container>;
		
		/** Create an instance of service */
		virtual Object<TService> operator()(
			const Container& container, ServiceStorage& storage) const {
			return getServiceInstance<Container>(container, storage, Extensions::IndexSequence);
		}
	
		/** Constructor **/
		ServiceDependencyInjectionFactory(Container& container) :
			dependencyDescriptors_(Extensions::getDependencyDescriptors(container)) { }
		
	private:
		/** Get instance of service */
		template <class ContainerType, std::size_t... I>
		Object<TService> getServiceInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			std::index_sequence<I...>) {
			return makeObject<TService>(Extensions::template
				getDependencyInstance<Extensions::DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
}

