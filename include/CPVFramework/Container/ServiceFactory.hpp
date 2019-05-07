#pragma once
#include <tuple>
#include <type_traits>
#include "./ServiceTraits.hpp"
#include "./ServiceDescriptorBase.hpp"
#include "./ServiceFactoryBase.hpp"

namespace cpv {
	/** Get instances of service by construct given implementation type with dependency injection */
	template <class TImplementation, class TService,
		class = std::enable_if_t<std::is_base_of_v<TService, TImplementation>>>
	class ServiceDependencyInjectionFactory : public ServiceFactoryBase<TService> {
	public:
		using DependencyTrait = ServiceDependencyTrait<TImplementation>;
		using DependencyTypes = typename DependencyTrait::DependencyTypes;
		static const constexpr std::size_t DependencyTypesSize = std::tuple_size_v<DependencyTypes>;
		
		/** Create an instance of service */
		virtual TService operator()(const Container& container, ServiceStorage& storage) const {
			// TODO: use ServiceCollectionTrait
			throw 1;
		}
		
		/** Constructor **/
		ServiceDependencyInjectionFactory(Container& container) :
			dependencyFactories_(this->getDependencyFactories(
				container, std::make_index_sequence<DependencyTypesSize>())) { }
		
	private:
		using ArrayType = std::array<ServiceDescriptorCollection, DependencyTypesSize>;
		
		template <class ContainerType, std::size_t... I>
		static ArrayType getDependencyFactories(ContainerType& container, std::index_sequence<I...>) {
			return ArrayType(container.getOrCreateEmptyDescriptors(
				typeid(std::tuple_element_t<I, DependencyTypes>))...);
		}
		
		ArrayType dependencyFactories_;
	};
	
	// TODO: add specialization for pointer types
}

