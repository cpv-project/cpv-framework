#pragma once
#include <memory>
#include <optional>
#include <functional>
#include <util/log.hh>
#include "../Exceptions/ContainerException.hpp"
#include "./Lifetime.hpp"

namespace cpv {
	class Container;

	/** Pointer of entry of a service, type is erased */
	using ServiceEntryPtr = std::unique_ptr<void, void(*)(void*)>;

	/** Entry of a service */
	template <class TService>
	struct ServiceEntry {
		Lifetime lifetime;
		mutable std::optional<TService> instance;
		std::function<TService(const Container&)> factory;

		/** Move self to a type erased pointer */
		ServiceEntryPtr toPtr() && {
			return ServiceEntryPtr(
				new ServiceEntry<TService>(std::move(*this)),
				[](void* ptr) { delete reinterpret_cast<ServiceEntry<TService>*>(ptr); });
		}

		/** Get a typed reference from a type erased pointer */
		static const ServiceEntry& fromPtr(const ServiceEntryPtr& ptr) {
			return *reinterpret_cast<const ServiceEntry<TService>*>(ptr.get());
		}

		/** Get the instance of the service */
		TService getInstance(const Container& container) const {
			try {
				if (lifetime == Lifetime::Singleton) {
					if constexpr (std::is_copy_constructible_v<TService>) {
						if (!instance.has_value()) {
							instance = factory(container);
						}
						return *instance;
					} else {
						throw ContainerException(CPV_CODEINFO,
							"can't create singleton service that's not copy constructible");
					}
				}
				return factory(container);
			} catch (...) {
				throw ContainerException(CPV_CODEINFO,
					"call factory error, service type:", typeid(TService).name(),
					", error:", std::current_exception());
			}
		}

		/** Default factory throws exception instead of crash program */
		static TService invalidFactory(const Container&) {
			throw ContainerException(CPV_CODEINFO, "invalid factory");
		}

		/** Constructor */
		ServiceEntry() :
			lifetime(Lifetime::Transient),
			instance(),
			factory(invalidFactory) { }

		/** Constructor */
		ServiceEntry(
			Lifetime lifetimeVal,
			std::optional<TService>&& instanceRef,
			std::function<TService(const Container&)>&& factoryRef) :
			lifetime(lifetimeVal),
			instance(std::move(instanceRef)),
			factory(std::move(factoryRef)) {
			if (!factory) {
				factory = invalidFactory;
			}
		}

		/** Disallow copy */
		ServiceEntry(const ServiceEntry&) = delete;
		ServiceEntry& operator=(const ServiceEntry&) = delete;

		/** Allow move */
		ServiceEntry(ServiceEntry&&) = default;
		ServiceEntry& operator=(ServiceEntry&&) = default;
	};
}

