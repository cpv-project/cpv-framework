#pragma once
#include <tuple>
#include <type_traits>
#include <memory>
#include <seastar/core/shared_ptr.hh>
#include "../Exceptions/ContainerException.hpp"
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
		template <class T, std::enable_if_t<!ServiceTypeTrait<T>::IsCollection, int> = 0>
		static T getDependencyInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			const ServiceDescriptorCollection& descriptors) {
			// make sure descriptor type is T
			static_assert(std::is_same_v<T, typename ServiceTypeTrait<T>::ActualType>);
			return container.template get<T>(storage, descriptors);
		}
		
		/** Get instance of single dependency type, for collection */
		template <class T, std::enable_if_t<ServiceTypeTrait<T>::IsCollection, int> = 0>
		static T getDependencyInstance(
			const ContainerType& container,
			ServiceStorage& storage,
			const ServiceDescriptorCollection& descriptors) {
			T collection;
			container.template getMany<T>(collection, storage, descriptors);
			return collection;
		}
		
		/** Get array contains service descriptors for each dependency type */
		static ArrayType getDependencyDescriptors(Container& container) {
			return getDependencyDescriptorsImpl(container, IndexSequence);
		}
		
	private:
		/** Get array contains service descriptors for each dependency type */
		template <std::size_t... I>
		static ArrayType getDependencyDescriptorsImpl(
			ContainerType& container, std::index_sequence<I...>) {
			return ArrayType({
				container.getOrCreateEmptyDescriptors(typeid(
					typename ServiceTypeTrait<DependencyType<I>>::ActualType))...
			});
		}
	};
	
	/** Get instances of service by construct given implementation type with dependency injection */
	template <class TService, class TImplementation, class = void /* for enable_if */>
	class ServiceDependencyInjectionFactory : public ServiceFactoryBase<TService> {
	public:
		static_assert(std::is_convertible_v<TImplementation, TService>);
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
			std::index_sequence<I...>) const {
			return TImplementation(Extensions::template
				getDependencyInstance<typename Extensions::template DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** std::unique_ptr version of ServiceDependencyInjectionFactory */
	template <class TService, class TImplementation>
	class ServiceDependencyInjectionFactory<
		std::unique_ptr<TService>,
		std::unique_ptr<TImplementation>> :
		public ServiceFactoryBase<std::unique_ptr<TService>> {
	public:
		static_assert(std::is_base_of_v<TService, TImplementation>);
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
			std::index_sequence<I...>) const {
			return std::make_unique<TImplementation>(Extensions::template
				getDependencyInstance<typename Extensions::template DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** seastar::shared_ptr version of ServiceDependencyInjectionFactory */
	template <class TService, class TImplementation>
	class ServiceDependencyInjectionFactory<
		seastar::shared_ptr<TService>,
		seastar::shared_ptr<TImplementation>> :
		public ServiceFactoryBase<seastar::shared_ptr<TService>> {
	public:
		static_assert(std::is_base_of_v<TService, TImplementation>);
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
			std::index_sequence<I...>) const {
			return seastar::make_shared<TImplementation>(Extensions::template
				getDependencyInstance<typename Extensions::template DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...);
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** Object version of ServiceDependencyInjectionFactory */
	template <class TService, class TImplementation>
	class ServiceDependencyInjectionFactory<
		Object<TService>,
		Object<TImplementation>> :
		public ServiceFactoryBase<Object<TService>> {
	public:
		static_assert(std::is_base_of_v<TService, TImplementation>);
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
			std::index_sequence<I...>) const {
			return makeObject<TImplementation>(Extensions::template
				getDependencyInstance<typename Extensions::template DependencyType<I>>(
				container, storage, dependencyDescriptors_[I])...).template cast<TService>();
		}
		
		typename Extensions::ArrayType dependencyDescriptors_;
	};
	
	/** Get instances of service by invoking custom function object */
	template <class TService, class TFunc, class = void /* for enable_if */>
	class ServiceFunctionFactory {
	public:
		ServiceFunctionFactory() = delete;
	};
	
	/** TFunc(Container, ServiceStorage) version of ServiceFunctionFactory */
	template <class TService, class TFunc>
	class ServiceFunctionFactory<
		TService,
		TFunc,
		std::enable_if_t<std::is_convertible_v<
			std::invoke_result_t<TFunc, const Container&, ServiceStorage&>, TService>>> :
		public ServiceFactoryBase<TService> {
	public:
		/** Create an instance of service */
		virtual TService operator()(
			const Container& container, ServiceStorage& storage) const {
			return func_(container, storage);
		}
		
		/** Constructor **/
		ServiceFunctionFactory(TFunc&& func) : func_(std::move(func)) { }
		
	private:
		TFunc func_;
	};
	
	/** TFunc(Container) version of ServiceFunctionFactory */
	template <class TService, class TFunc>
	class ServiceFunctionFactory<
		TService,
		TFunc,
		std::enable_if_t<std::is_convertible_v<
			std::invoke_result_t<TFunc, const Container&>, TService>>> :
		public ServiceFactoryBase<TService> {
	public:
		/** Create an instance of service */
		virtual TService operator()(const Container& container, ServiceStorage&) const {
			return func_(container);
		}
		
		/** Constructor **/
		ServiceFunctionFactory(TFunc&& func) : func_(std::move(func)) { }
		
	private:
		TFunc func_;
	};
	
	/** TFunc() version of ServiceFunctionFactory */
	template <class TService, class TFunc>
	class ServiceFunctionFactory<
		TService,
		TFunc,
		std::enable_if_t<std::is_convertible_v<
			std::invoke_result_t<TFunc>, TService>>> :
		public ServiceFactoryBase<TService> {
	public:
		/** Create an instance of service */
		virtual TService operator()(const Container&, ServiceStorage&) const {
			return func_();
		}
		
		/** Constructor **/
		ServiceFunctionFactory(TFunc&& func) : func_(std::move(func)) { }
		
	private:
		TFunc func_;
	};
	
	/** Factory throws exception with given message */
	template <class TService>
	class ServiceExceptionFactory : public ServiceFactoryBase<TService> {
	public:
		/** Throw the exception */
		virtual TService operator()(const Container&, ServiceStorage&) const {
			throw ContainerException(CPV_CODEINFO,
				"get instance of service type [", typeid(TService).name(),
				"] error:", message_);
		}
		
		/** Constructor **/
		ServiceExceptionFactory(const char* message) : message_(message) { }
		
	private:
		const char* message_;
	};
}
