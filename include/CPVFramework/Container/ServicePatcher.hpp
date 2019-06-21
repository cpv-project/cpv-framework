#pragma once
#include <typeindex>
#include "./ServiceDescriptor.hpp"
#include "./ServiceFactory.hpp"
#include "./ServiceStorage.hpp"

namespace cpv {
	/** Static class used to patch services registered in container */
	template <class TService>
	struct ServicePatcher {
		/**
		 * Patch registered services by given a wrapper function,
		 * the lifetime of wrapper instance will be same as original.
		 */
		template <class TContainer, class TFunc>
		static std::size_t patch(TContainer& container, TFunc func) {
			ServiceDescriptorCollection descriptors = container.getDescriptors(typeid(TService));
			if (descriptors.get() == nullptr) {
				return 0;
			}
			for (std::size_t i = 0; i < descriptors->size(); ++i) {
				ServiceDescriptorPtr original = std::move((*descriptors)[i]);
				ServiceLifetime lifetime = ServiceDescriptor<TService>::cast(original).getLifetime();
				(*descriptors)[i] = ServiceDescriptor<TService>::create(
					std::optional<TService>(),
					getPatchedServiceFactory<TContainer>(std::move(original), std::move(func)),
					lifetime); // replace inplace to keep dependency injection factory work
			}
			return descriptors->size();
		}
		
	private:
		/** Get a service factory that invoke original descriptor and wrap result with func */
		template <class TContainer, class TFunc,
			std::enable_if_t<std::is_convertible_v<
				std::invoke_result_t<TFunc,
					const TContainer&, ServiceStorage&, TService&&>, TService>, int> = 0>
		static std::unique_ptr<ServiceFactoryBase<TService>> getPatchedServiceFactory(
			ServiceDescriptorPtr&& original, TFunc&& func) {
			auto patchFunc = [o=std::move(original), f=std::move(func)] (
				const TContainer& c, ServiceStorage& s) {
					return f(c, s, ServiceDescriptor<TService>::cast(o).getInstance(c, s));
				};
			return std::make_unique<ServiceFunctionFactory<
				TService, decltype(patchFunc)>>(std::move(patchFunc));
		}
		
		/** Get a service factory that invoke original descriptor and wrap result with func */
		template <class TContainer, class TFunc,
			std::enable_if_t<std::is_convertible_v<
				std::invoke_result_t<TFunc, const TContainer&, TService&&>, TService>, int> = 0>
		static std::unique_ptr<ServiceFactoryBase<TService>> getPatchedServiceFactory(
			ServiceDescriptorPtr&& original, TFunc&& func) {
			auto patchFunc = [o=std::move(original), f=std::move(func)] (
				const TContainer& c, ServiceStorage& s) {
					return f(c, ServiceDescriptor<TService>::cast(o).getInstance(c, s));
				};
			return std::make_unique<ServiceFunctionFactory<
				TService, decltype(patchFunc)>>(std::move(patchFunc));
		}
		
		/** Get a service factory that invoke original descriptor and wrap result with func */
		template <class TContainer, class TFunc,
			std::enable_if_t<std::is_convertible_v<
				std::invoke_result_t<TFunc, TService&&>, TService>, int> = 0>
		static std::unique_ptr<ServiceFactoryBase<TService>> getPatchedServiceFactory(
			ServiceDescriptorPtr&& original, TFunc&& func) {
			auto patchFunc = [o=std::move(original), f=std::move(func)] (
				const TContainer& c, ServiceStorage& s) {
					return f(ServiceDescriptor<TService>::cast(o).getInstance(c, s));
				};
			return std::make_unique<ServiceFunctionFactory<
				TService, decltype(patchFunc)>>(std::move(patchFunc));
		}
	};
}

