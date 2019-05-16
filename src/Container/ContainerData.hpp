#pragma once
#include <typeindex>
#include <unordered_map>
#include <CPVFramework/Container/ServiceDescriptorBase.hpp>

namespace cpv {
	/** Members of Container */
	class ContainerData {
	public:
		/** { type: [ descriptor... ] } */
		std::unordered_map<std::type_index, ServiceDescriptorCollection> descriptorsMap;
		/** Built-in service storage for service with ServiceLifetime::StoragePresistent */
		ServiceStorage storage;
		
		ContainerData() : descriptorsMap(), storage() { }
	};
}

