#pragma once
#include <memory>
#include <seastar/core/shared_ptr.hh>
#include "../Allocators/StackAllocator.hpp"

namespace cpv {
	/** The base class of ServiceDescriptor<TService> */
	class ServiceDescriptorBase {
	public:
		/** Virtual destructor */
		virtual ~ServiceDescriptorBase() = default;
	};
	
	/** The pointer type used to store all service descriptor in container */
	using ServiceDescriptorPtr = std::unique_ptr<ServiceDescriptorBase>;
	
	/** The collection type of service descriptor */
	using ServiceDescriptorCollection = seastar::shared_ptr<StackAllocatedVector<ServiceDescriptorPtr, 1>>;
}

