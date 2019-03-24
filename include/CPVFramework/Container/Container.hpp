#pragma once
#include <typeindex>
#include <seastar/core/shared_ptr.hh>
#include "../Allocators/StackAllocator.hpp"
#include "./ServiceDescriptor.hpp"

namespace cpv {
	/** Members of Container */
	class ContainerData;
	
	/**
	 * Dependency injection container.
	 * It can't use across cpu cores, please create a container for each cpu core.
	 */
	class Container {
	public:
		using ServiceDescriptorsType = StackAllocatedVector<ServiceDescriptorPtr, 1>;
		
		
		
	private:
		/** Associate a descriptor to given service type */
		void add(const std::type_index& serviceType, ServiceDescriptorPtr&& serviceDescriptor);
		
		/** Get all descriptors associated to given service type, may return an empty list */
		const ServiceDescriptorsType& getDescriptors(const std::type_index& serviceType) const&;
		
		/** Get all descriptors associated to given service type, may return an empty list */
		ServiceDescriptorsType& getDescriptors(const std::type_index& serviceType) &;
		
	private:
		seastar::shared_ptr<ContainerData> data_;
	};
}
 
