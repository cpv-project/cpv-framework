#include <CPVFramework/Container/Container.hpp>
#include "ContainerImpl.hpp"

namespace cpv {
	/** Create a dependency injection container */
	seastar::shared_ptr<Container> Container::create() {
		return seastar::make_shared<ContainerImpl>();
	}
}

