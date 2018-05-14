#pragma once
#include "Container.hpp"

namespace cpv {
	/** Provide basic aop support for container */
	class Aop {
	public:
		/** Patch service entries that already added to container */
		static void patch(
			Container& container,
			const std::type_index& serviceType,
			const std::function<void(ServiceEntryPtr&)>& patchFunc);

		/** Patch service entries that already added to container */
		template <class TService>
		static void patch(
			Container& container,
			const std::function<TService(const Container&, TService&&)>& patchFunc) {
			patch(container, typeid(TService), [&patchFunc] (ServiceEntryPtr& serviceEntry) {
				auto typedEntry = reinterpret_cast<ServiceEntry<TService>*>(serviceEntry.get());
				if (typedEntry->lifetime == Lifetime::Singleton &&
					typedEntry->instance.has_value()) {
					// patch instance
					if constexpr (std::is_copy_constructible_v<TService>) {
						typedEntry->factory = [
							instance=std::move(*typedEntry->instance),
							patchFunc=std::move(patchFunc)]
							(const Container& container) mutable {
							return patchFunc(container, std::move(instance));
						};
						typedEntry->instance.reset();
					} else {
						throw ContainerException(CPV_CODEINFO,
							"can't patch singleton service that's not copy constructible");
					}
				} else {
					// patch factory
					typedEntry->factory = [
						factory=std::move(typedEntry->factory),
						patchFunc=std::move(patchFunc)]
						(const Container& container) {
						return patchFunc(container, factory(container));
					};
				}
			});
		}
	};
}

