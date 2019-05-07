#pragma once
#include <typeindex>
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
		
	private:
		/** Associate a descriptor to given service type */
		void add(const std::type_index& serviceType, ServiceDescriptorPtr&& serviceDescriptor);
		
		/** Get all descriptors associated to given service type, return null pointer if not registered */
		const ServiceDescriptorCollection& getDescriptors(const std::type_index& serviceType) const&;
		
		/** Get all descriptors associated to given service type, return empty list if not registered */
		ServiceDescriptorCollection& getOrCreateEmptyDescriptors(const std::type_index& serviceType) &;
		
	private:
		seastar::shared_ptr<ContainerData> data_;
	};
}

