#pragma once
#include "../Exceptions/ContainerException.hpp"
#include "./ServiceLifetime.hpp"
#include "./ServiceFactoryBase.hpp"
#include "./ServiceDescriptorBase.hpp"
#include "./ServiceStorage.hpp"

namespace cpv {
	class Container;
	
	/** Manage the factory and the persistent instance for a given service implementation */
	template <class TService>
	class ServiceDescriptor : public ServiceDescriptorBase {
	public:
		/** Get an instance of this service */
		TService getInstance(const Container& container, ServiceStorage& storage) const {
			try {
				if (lifetime_ == ServiceLifetime::Persistent) {
					if constexpr (std::is_copy_constructible_v<TService>) {
						if (CPV_UNLIKELY(!instance_.has_value())) {
							instance_ = (*factory_)(container, storage);
						}
						return *instance_;
					} else {
						throw ContainerException(CPV_CODEINFO,
							"get instance of service type [", typeid(TService).name(),
							"] error: lifetime is persistent but not copy constructible");
					}
				} else if (lifetime_ == ServiceLifetime::Transient) {
					return (*factory_)(container, storage);
				} else if (lifetime_ == ServiceLifetime::StoragePersistent) {
					if constexpr (std::is_copy_constructible_v<TService>) {
						std::uintptr_t key = reinterpret_cast<std::uintptr_t>(this);
						std::any anyInstance = storage.get(key);
						if (CPV_UNLIKELY(!anyInstance.has_value())) {
							TService instance = (*factory_)(container, storage);
							storage.set(key, instance);
							return instance;
						} else {
							return std::any_cast<TService>(anyInstance);
						}
					} else {
						throw ContainerException(CPV_CODEINFO,
							"get instance of service type [", typeid(TService).name(),
							"] error: lifetime is storage persistent but not copy constructible");
					}
				} else {
					throw ContainerException(CPV_CODEINFO,
						"get instance of service type [", typeid(TService).name(),
						"] error: unsupported lifetime type (",
						static_cast<std::size_t>(lifetime_), ")");
				}
			} catch (const ContainerException&) {
				throw;
			} catch (...) {
				throw ContainerException(CPV_CODEINFO,
					"get instance of service type [", typeid(TService).name(),
					"] error:", std::current_exception());
			}
		}
		
		/** Get the lifetime of this service */
		ServiceLifetime getLifetime() const {
			return lifetime_;
		}
		
		/** Create a service descriptor pointer */
		static ServiceDescriptorPtr create(
			std::optional<TService>&& instance,
			std::unique_ptr<ServiceFactoryBase<TService>>&& factory,
			ServiceLifetime lifetime) {
			return ServiceDescriptorPtr(new ServiceDescriptor<TService>(
				std::move(instance), std::move(factory), lifetime));
		}
		
		/** Cast from a service descriptor pointer, notice no type check will perform */
		static const ServiceDescriptor<TService>& cast(const ServiceDescriptorPtr& ptr) {
			return *static_cast<ServiceDescriptor<TService>*>(ptr.get());
		}
		
	private:
		/** Constructor */
		ServiceDescriptor(
			std::optional<TService>&& instance,
			std::unique_ptr<ServiceFactoryBase<TService>>&& factory,
			ServiceLifetime lifetime) :
			instance_(std::move(instance)),
			factory_(std::move(factory)),
			lifetime_(lifetime) { }
		
	private:
		mutable std::optional<TService> instance_; // only for ServiceLifetime::Persistent
		std::unique_ptr<ServiceFactoryBase<TService>> factory_;
		ServiceLifetime lifetime_;
	};
}

