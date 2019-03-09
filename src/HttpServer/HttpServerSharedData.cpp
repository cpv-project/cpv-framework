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
				"request_served",
				[this] { return metricData.request_served; },
				seastar::metrics::description("The total number of http request served"),
				labels),
			seastar::metrics::make_derive(
				"read_errors",
				[this] { return metricData.read_errors; },
				seastar::metrics::description("The total number of errors while reading from client"),
				labels),
			seastar::metrics::make_derive(
				"write_errors",
				[this] { return metricData.write_errors; },
				seastar::metrics::description("The total number of errors while writing to client"),
				labels)
		});
	}
}

