#pragma once
#include "Module.hpp"

namespace cpv {
	/**
	 * Provides default services such as logger,
	 * this module will automatically register from Application.
	 */
	class DefaultModule : public Module {
	public:
		using Module::Module;

		/** Register default services */
		seastar::future<> registerServices(Container& container) override;
	};
}

