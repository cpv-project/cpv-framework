#include <core/prometheus.hh>
#include <CPVFramework/Module/PrometheusModule.hpp>
#include <CPVFramework/Utility/JsonUtils.hpp>

namespace cpv {
	/** Register prometheus handler */
	seastar::future<> PrometheusModule::registerRoutes(
		const seastar::shared_ptr<const Container>&,
		httpd::http_server& server) {
		seastar::prometheus::config ctx;
		ctx.metric_help = configuration_->value<std::string>("prometheus.metric_help", ctx.metric_help);
		ctx.hostname = configuration_->value<std::string>("prometheus.hostname", ctx.hostname);
		ctx.prefix = configuration_->value<std::string>("prometheus.prefix", ctx.prefix);
		return seastar::prometheus::add_prometheus_routes(server, ctx);
	}
}

