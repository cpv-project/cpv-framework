#include <CPVFramework/Container/ServiceStorage.hpp>

namespace cpv {
	/** Get the service instance with associated key, may return empty object */
	std::any ServiceStorage::get(std::uintptr_t key) const {
		auto it = instances_.find(key);
		if (CPV_UNLIKELY(it == instances_.end())) {
			return std::any();
		}
		return it->second;
	}
	
	/** Set the service instance with associated key */
	void ServiceStorage::set(std::uintptr_t key, std::any&& value) {
		instances_.insert_or_assign(key, std::move(value));
	}

	/** Clear all instances store in this storage */
	void ServiceStorage::clear() {
		instances_.clear();
	}
}

