#pragma once
#include "Module.hpp"

namespace cpv {
	/**
	 * Export prometheus interface on "/metrics"
	 */
	class PrometheusModule : public Module {
	public:
		using Module::Module;

		/** Register prometheus handler */
		seastar::future<> registerRoutes(
			const seastar::shared_ptr<const Container>& container,
			httpd::http_server& server) override;
	};
}

