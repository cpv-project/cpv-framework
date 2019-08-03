#include <seastar/core/metrics.hh>
#include <CPVFramework/Utility/StringUtils.hpp>
#include "./Handlers/HttpServerRequestRealLastHandler.hpp"
#include "./HttpServerSharedData.hpp"

namespace cpv {
	/** Constructor **/
	HttpServerConnectionsWrapper::HttpServerConnectionsWrapper() :
		value() { }
	
	/** Constructor */
	HttpServerSharedData::HttpServerSharedData(
		const HttpServerConfiguration& configurationVal,
		const seastar::shared_ptr<Logger>& loggerVal,
		HttpServerRequestHandlerCollection&& handlersVal,
		seastar::weak_ptr<HttpServerConnectionsWrapper>&& connectionsWrapperVal) :
		configuration(configurationVal),
		logger(loggerVal),
		handlers((
			// add real last handler to handler list
			handlersVal.emplace_back(std::make_unique<HttpServerRequestRealLastHandler>()),
			std::move(handlersVal))),
		connectionsWrapper(std::move(connectionsWrapperVal)),
		metricData(),
		metricGroups_() {
		// initialize metric groups
		static thread_local std::size_t ServiceId = 0;
		std::vector<seastar::metrics::label_instance> labels;
		labels.emplace_back("service", joinString("", "cpv-http-server-", ServiceId++));
		metricGroups_.add_group("cpv-http-server", {
			seastar::metrics::make_derive(
				"total_connections",
				[this] { return metricData.total_connections; },
				seastar::metrics::description("The total number of connections accepted"),
				labels),
			seastar::metrics::make_gauge(
				"current_connections",
				[this] { return metricData.current_connections; },
				seastar::metrics::description("The current number of open connections"),
				labels),
			seastar::metrics::make_derive(
				"request_received",
				[this] { return metricData.request_received; },
				seastar::metrics::description("The total number of http request received"),
				labels),
			seastar::metrics::make_derive(
				"request_errors",
				[this] { return metricData.request_errors; },
				seastar::metrics::description("The total number of errors while handling http request"),
				labels),
			seastar::metrics::make_derive(
				"request_timeout_errors",
				[this] { return metricData.request_timeout_errors; },
				seastar::metrics::description("The total number of timeout errors while handling http request"),
				labels),
			seastar::metrics::make_derive(
				"request_initial_size_errors",
				[this] { return metricData.request_initial_size_errors; },
				seastar::metrics::description("The total number of initial size errors while handling http request"),
				labels),
			seastar::metrics::make_derive(
				"request_invalid_format_errors",
				[this] { return metricData.request_invalid_format_errors; },
				seastar::metrics::description("The total number of invalid format errors while handling http request"),
				labels),
		});
	}
}

