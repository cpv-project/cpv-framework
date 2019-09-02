#pragma once
#include <any>
#include "../Allocators/StackAllocator.hpp"

namespace cpv {
	/** The storage used to store instance of services with ServiceLifetime::StoragePersistent */
	class ServiceStorage {
	public:
		/** Get the service instance with associated key, may return empty object */
		std::any get(std::uintptr_t key) const;
		
		/** Set the service instance with associated key */
		void set(std::uintptr_t key, std::any&& value);

		/** Clear all instances store in this storage */
		void clear();
        
	private:
		/** Store service instances with lifetime StoragePersistent, key is pointer of descriptor */
		StackAllocatedUnorderedMap<std::uintptr_t, std::any, 16> instances_;
	};
}

