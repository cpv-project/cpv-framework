#include <CPVFramework/Container/Aop.hpp>

namespace cpv {
	/** Patch service entries that already added to container */
	void Aop::patch(
		Container& container,
		const std::type_index& serviceType,
		const std::function<void(ServiceEntryPtr&)>& patchFunc) {
		auto serviceEntries = container.remove(serviceType);
		for (auto& serviceEntry : serviceEntries) {
			patchFunc(serviceEntry);
			container.add(serviceType, std::move(serviceEntry));
		}
	}
}

