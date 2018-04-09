#pragma once
#include "./Container.hpp"

namespace cpv {
	/**
	 * Inject constructor arguments from external.
	 * For example: container.add<shared_ptr<Base>, shared_ptr<Injector<Derived, A, B>>>();
	 */
	template <class TImplementation, class... Args>
	struct Injector : public TImplementation {
		Injector(const Container& container) :
			TImplementation(container.get<Args>()...) { }
	};
}

