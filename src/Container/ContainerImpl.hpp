#pragma once
#include <unordered_map>
#include <CPVFramework/Container/Container.hpp>

namespace cpv {
	/** Implementation of dependency injection container */
	class ContainerImpl : public Container {
	public:
		/** Add a service entry */
		void add(const std::type_index& serviceType, ServiceEntryPtr&& serviceEntry) override;

		/** Remove entries of the service, return how many entries removed */
		std::size_t remove(const std::type_index& serviceType) override;

		/** Get entires of the service, may return an empty list */
		const std::vector<ServiceEntryPtr>& getEntries(const std::type_index& serviceType) const override;

		/** Constructor */
		ContainerImpl();

	private:
		std::unordered_map<std::type_index, std::vector<ServiceEntryPtr>> entries_;
	};
}

