#include <CPVFramework/Container/Container.hpp>

namespace cpv {
	/** Defines members of Container */
	struct ContainerData {
		std::unordered_map<std::type_index, std::vector<ServiceEntryPtr>> entries;

		ContainerData() : entries() { }
	};

	/** Add a single service entry */
	void Container::add(const std::type_index& serviceType, ServiceEntryPtr&& serviceEntry) {
		auto it = data_->entries.find(serviceType);
		if (it == data_->entries.end()) {
			it = data_->entries.emplace(serviceType, std::vector<ServiceEntryPtr>()).first;
		}
		it->second.emplace_back(std::move(serviceEntry));
	}

	/** Get entires of the service, may return an empty list */
	const std::vector<ServiceEntryPtr>& Container::getEntries(
		const std::type_index& serviceType) const {
		static thread_local std::vector<ServiceEntryPtr> empty;
		auto it = data_->entries.find(serviceType);
		if (it == data_->entries.end()) {
			return empty;
		}
		return it->second;
	}

	/** Constructor */
	Container::Container() :
		data_(seastar::make_shared<ContainerData>()) {
		// add self
		add<const Container>([](const Container& container) { return container; });
	}
}

