#include <CPVFramework/Container/Container.hpp>
#include "ContainerData.hpp"

namespace cpv {
	/** Constructor **/
	Container::Container() :
		data_(seastar::make_shared<ContainerData>()) { }
	
	/** Associate a descriptor to given service type */
	void Container::addDescriptor(
		const std::type_index& serviceType, ServiceDescriptorPtr&& serviceDescriptor) {
		auto& descriptors = getOrCreateEmptyDescriptors(serviceType);
		descriptors->emplace_back(std::move(serviceDescriptor));
	}
	
	/** Get all descriptors associated to given service type, return null pointer if not registered */
	const ServiceDescriptorCollection& Container::getDescriptors(
		const std::type_index& serviceType) const& {
		static thread_local ServiceDescriptorCollection empty;
		auto it = data_->descriptorsMap.find(serviceType);
		if (it != data_->descriptorsMap.end()) {
			return it->second;
		}
		return empty;
	}
	
	/** Get all descriptors associated to given service type, return empty list if not registered */
	ServiceDescriptorCollection& Container::getOrCreateEmptyDescriptors(
		const std::type_index& serviceType) & {
		auto it = data_->descriptorsMap.find(serviceType);
		if (it == data_->descriptorsMap.end()) {
			it = data_->descriptorsMap.emplace(serviceType, ServiceDescriptorCollection()).first;
		}
		return it->second;
	}
}

