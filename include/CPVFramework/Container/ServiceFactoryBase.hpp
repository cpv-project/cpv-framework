#pragma once

namespace cpv {
	class Container;
	class ServiceStorage;
	
	/** Base class of service factory */
	template <class T>
	class ServiceFactoryBase {
	public:
		/**
		 * Create an instance of service.
		 * The container and storage arguments are for resolving dependencies,
		 * the lifetime of instance should not be managed here.
		 */
		virtual T operator()(const Container& container, ServiceStorage& storage) const = 0;
		
		/** Virtual destructor */
		virtual ~ServiceFactoryBase() = default;
	};
}

