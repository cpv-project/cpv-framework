#include <CPVFramework/Container/Container.hpp>

namespace cpv {
	/** Members of Container */
	class ContainerData {
	public:
		/** { type: [ descriptor... ] } */
		std::unordered_map<std::type_index, ServiceDescriptorCollection> descriptorsMap;
		/** Built-in service storage for service with ServiceLifetime::StoragePersistent */
		ServiceStorage storage;
		
		ContainerData() : descriptorsMap(), storage() { }
	};
	
	/** Constructor **/
	Container::Container() :
		data_(seastar::make_shared<ContainerData>()) { }
	
	/** Constructor for null container */
	Container::Container(std::nullptr_t) :
		data_(nullptr) { }
	
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
			it = data_->descriptorsMap.emplace(serviceType,
				seastar::make_shared<ServiceDescriptorCollection::element_type>()).first;
		}
		return it->second;
	}
	
	/** Get built-in service storage */
	ServiceStorage& Container::getBuiltinStorage() const& {
		return data_->storage;
	}
}

