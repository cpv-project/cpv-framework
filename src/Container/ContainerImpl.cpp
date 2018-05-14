#include "ContainerImpl.hpp"

namespace cpv {
	/** Add a single service entry */
	void ContainerImpl::add(const std::type_index& serviceType, ServiceEntryPtr&& serviceEntry) {
		auto it = entries_.find(serviceType);
		if (it == entries_.end()) {
			it = entries_.emplace(serviceType, std::vector<ServiceEntryPtr>()).first;
		}
		it->second.emplace_back(std::move(serviceEntry));
	}

	/** Remove entries of the service, return how many entries removed */
	std::size_t ContainerImpl::remove(const std::type_index& serviceType) {
		auto it = entries_.find(serviceType);
		if (it == entries_.end()) {
			return 0;
		}
		std::size_t removed = it->second.size();
		entries_.erase(it);
		return removed;
	}

	/** Get entires of the service, may return an empty list */
	const std::vector<ServiceEntryPtr>& ContainerImpl::getEntries(
		const std::type_index& serviceType) const {
		static thread_local std::vector<ServiceEntryPtr> empty;
		auto it = entries_.find(serviceType);
		if (it == entries_.end()) {
			return empty;
		}
		return it->second;
	}

	/** Constructor */
	ContainerImpl::ContainerImpl() : entries_() { }
}

