#pragma once
#include <memory>
#include "../Exceptions/ContainerException.hpp"
#include "./ServiceLifetime.hpp"
#include "./ServiceFactory.hpp"

namespace cpv {
	class Container;
	class ServiceStorage;
	
	/** The base class of ServiceDescriptor<TService> */
	class ServiceDescriptorBase {
	public:
		/** Virtual destructor */
		virtual ~ServiceDescriptorBase() = default;
	};
	
	/** The pointer type used to store all service descriptor in container */
	using ServiceDescriptorPtr = std::unique_ptr<ServiceDescriptorBase>;
	
	/** Manage the factory and the presistent instance for a given service implementation */
	template <class TService>
	class ServiceDescriptor : public ServiceDescriptorBase {
	public:
		/** Get an instance of given service */
		TService getInstance(const Container& container, ServiceStorage& storage) const {
			try {
				if (lifetime_ == ServiceLifetime::Presistent) {
					if constexpr (std::is_copy_constructible_v<TService>) {
						if (!instance_.has_value()) {
							instance_ = factory_(container, storage);
						}
						return *instance_;
					} else {
						throw ContainerException(CPV_CODEINFO,
							"get instance of service type [", typeid(TService).name(),
							"] error: lifetime is presistent but not copy constructible");
					}
				} else if (lifetime_ == ServiceLifetime::Transient) {
					return factory_(container, storage);
				} else if (lifetime_ == ServiceLifetime::StoragePresistent) {
					if constexpr (std::is_copy_constructible_v<TService>) {
						// TODO
						throw 1;
					} else {
						throw ContainerException(CPV_CODEINFO,
							"get instance of service type [", typeid(TService).name(),
							"] error: lifetime is storage presistent but not copy constructible");
					}
				} else {
					throw ContainerException(CPV_CODEINFO,
						"get instance of service type [", typeid(TService).name(),
						"] error: unsupported lifetime type (",
						static_cast<std::size_t>(lifetime_), ")");
				}
			} catch (ContainerException) {
				throw;
			} catch (...) {
				throw ContainerException(CPV_CODEINFO,
					"get instance of service type [", typeid(TService).name(),
					"] error:", std::current_exception());
			}
		}
		
		/** Create a service descriptor pointer */
		static ServiceDescriptorPtr create(
			std::optional<TService>&& instance,
			std::unique_ptr<ServiceFactoryBase<TService>>&& factory,
			ServiceLifetime lifetime);
		
		/** Cast from a service descriptor pointer, notice no type check will perform */
		static const ServiceDescriptor<TService>& cast(const ServiceDescriptorPtr& ptr);
		
	private:
		/** Constructor */
		ServiceDescriptor(
			std::optional<TService>&& instance,
			std::unique_ptr<ServiceFactoryBase<TService>>&& factory,
			ServiceLifetime lifetime);
		
	private:
		mutable std::optional<TService> instance_; // only for ServiceLifetime::Presistent
		std::unique_ptr<ServiceFactoryBase<TService>> factory_;
		ServiceLifetime lifetime_;
	};
}

