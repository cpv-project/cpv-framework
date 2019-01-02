#pragma once
#include <typeindex>
#include <vector>
#include <seastar/core/shared_ptr.hh>
#include "./ServiceEntry.hpp"
#include "./ServiceTraits.hpp"

namespace cpv {
	/**
	 * Interface of dependency injection container.
	 * It can't use across cpu cores, please create a container for each cpu core.
	 */
	class Container {
	public:
		/** Add a service entry */
		virtual void add(const std::type_index& serviceType, ServiceEntryPtr&& serviceEntry) = 0;

		/** Remove entries of the service, return removed entries */
		virtual std::vector<ServiceEntryPtr> remove(const std::type_index& serviceType) = 0;

		/** Get entires of the service, may return an empty list */
		virtual const std::vector<ServiceEntryPtr>& getEntries(const std::type_index& serviceType) const& = 0;

		/** Virtual destructor */
		virtual ~Container() = default;

		/** Add a service entry by it's instance as singleton */
		template <class TService>
		void add(const TService& instance) {
			add(typeid(TService), ServiceEntry<TService>(
				Lifetime::Singleton,
				std::optional<TService>(instance),
				nullptr).toPtr());
		}

		/** Add a service entry by it's instance as singleton */
		template <class TService>
		void add(TService&& instance) {
			add(typeid(TService), ServiceEntry<TService>(
				Lifetime::Singleton,
				std::optional<TService>(std::move(instance)),
				nullptr).toPtr());
		}

		/** Add a service entry by it's factory */
		template <class TService>
		void add(
			const std::function<TService(const Container&)>& factory,
			Lifetime lifetime = Lifetime::Transient) {
			add(typeid(TService), ServiceEntry<TService>(
				lifetime,
				std::optional<TService>(),
				factory).toPtr());
		}

		/** Add a service entry by it's factory */
		template <class TService>
		void add(
			std::function<TService(const Container&)>&& factory,
			Lifetime lifetime = Lifetime::Transient) {
			add(typeid(TService), ServiceEntry<TService>(
				lifetime,
				std::optional<TService>(),
				std::move(factory)).toPtr());
		}

		/** Add a service entry by it's implementation type */
		template <class TService, class TImplementation>
		void add(Lifetime lifetime = Lifetime::Transient) {
			add(typeid(TService), ServiceEntry<TService>(
				lifetime,
				std::optional<TService>(),
				[] (const Container& container) {
					return ServiceFactoryTrait<TService, TImplementation>()(container);
				}).toPtr());
		}

		/** Remove entries of the service, return removed entries */
		template <class TService>
		std::vector<ServiceEntryPtr> remove() {
			return remove(typeid(TService));
		}

		/** Get service instance(s) */
		template <class TService>
		TService get() const {
			if constexpr (ServiceCollectionTrait<TService>::isCollection) {
				// get multiple instances
				using ElementType = typename ServiceCollectionTrait<TService>::ElementType;
				auto& entires = getEntries(typeid(ElementType));
				TService result;
				for (const auto& entryPtr : entires) {
					auto& entry = ServiceEntry<ElementType>::fromPtr(entryPtr);
					ServiceCollectionTrait<TService>::add(result, entry.getInstance(*this));
				}
				return result;
			} else {
				// get single instance
				auto& entires = getEntries(typeid(TService));
				if (entires.empty()) {
					throw ContainerException(CPV_CODEINFO,
						"no service entry found, type:", typeid(TService).name());
				} else if (entires.size() > 1) {
					throw ContainerException(CPV_CODEINFO,
						"more than 1 service entry found, type:", typeid(TService).name());
				}
				return ServiceEntry<TService>::fromPtr(entires[0]).getInstance(*this);
			}
		}

		/** Apply function to each service instances */
		template <class TService>
		void each(const std::function<void(TService)>& func) const {
			auto& entires = getEntries(typeid(TService));
			for (const auto& entryPtr : entires) {
				auto& entry = ServiceEntry<TService>::fromPtr(entryPtr);
				func(entry.getInstance(*this));
			}
		}

		/** Create a dependency injection container */
		static seastar::shared_ptr<Container> create();
	};
}

