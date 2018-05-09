#include "ContainerImpl.hpp"

namespace cpv {
	/** Add a single service entry */
	void ContainerImpl::add(const std::type_index& serviceType, ServiceEntryPtr&& serviceEntry) {
		auto it = entries.find(serviceType);
		if (it == entries.end()) {
			it = entries.emplace(serviceType, std::vector<ServiceEntryPtr>()).first;
		}
		it->second.emplace_back(std::move(serviceEntry));
	}

	/** Get entires of the service, may return an empty list */
	const std::vector<ServiceEntryPtr>& ContainerImpl::getEntries(
		const std::type_index& serviceType) const {
		static thread_local std::vector<ServiceEntryPtr> empty;
		auto it = entries.find(serviceType);
		if (it == entries.end()) {
			return empty;
		}
		return it->second;
	}

	/** Constructor */
	ContainerImpl::ContainerImpl() : entries() { }
}

