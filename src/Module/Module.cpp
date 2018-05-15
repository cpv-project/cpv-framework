#include <CPVFramework/Module/Module.hpp>

namespace cpv {
	/** Register services to dependency injection container, do nothing by default */
	seastar::future<> Module::registerServices(Container&) {
		return seastar::make_ready_future<>();
	}

	/** Initialize registered services, do nothing by default */
	seastar::future<> Module::initializeServices(const Container&) {
		return seastar::make_ready_future<>();
	}

	/** Register http handlers for http server, do nothing by default */
	seastar::future<> Module::registerRoutes(
		const seastar::shared_ptr<const Container>&,
		httpd::http_server&) {
		return seastar::make_ready_future<>();
	}

	/** Constructor */
	Module::Module(const seastar::shared_ptr<const Json>& configuration) :
		configuration_(configuration) { }
}

