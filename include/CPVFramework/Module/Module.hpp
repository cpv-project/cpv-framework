#pragma once
#include <seastar/core/shared_ptr.hh>
#include <seastar/core/future.hh>
#include "../Container/Container.hpp"
#include "../Utility/JsonUtilsSlim.hpp"
#include "../Http/Server.hpp"

namespace cpv {
	/**
	 * The base class of modules.
	 * A module will setup with following calls in order:
	 * - registerServices
	 * - initializeServices
	 * - registerRoutes
	 */
	class Module {
	public:
		/** Register services to dependency injection container, do nothing by default */
		virtual seastar::future<> registerServices(Container& container);

		/** Initialize registered services, do nothing by default */
		virtual seastar::future<> initializeServices(const Container& container);

		/** Register http handlers for http server, do nothing by default */
		virtual seastar::future<> registerRoutes(
			const seastar::shared_ptr<const Container>& container,
			httpd::http_server& server);

		/** Constructor */
		explicit Module(const seastar::shared_ptr<const Json>& configuration);

		/** Virtual destructor */
		virtual ~Module() = default;

	protected:
		seastar::shared_ptr<const Json> configuration_;
	};
}

