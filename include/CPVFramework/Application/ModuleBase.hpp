#pragma once
#include <seastar/core/future.hh>
#include "../Container/Container.hpp"
#include "./ApplicationState.hpp"

namespace cpv {
	/** The base class of modules */
	class ModuleBase {
	public:
		/** Do some work for given application state */
		virtual seastar::future<> handle(Container& container, ApplicationState state) = 0;

		/** Virtual destructor */
		virtual ~ModuleBase() = default;
	};
}

