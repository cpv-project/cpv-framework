#include <vector>
#include <core/metrics.hh>
#include <CPVFramework/Utility/StringUtils.hpp>
#include "HttpClientMetricsData.hpp"

namespace cpv {
	/** Constructor */
	HttpClientMetricsData::HttpClientMetricsData(const std::string& hostHeader) :
		connections_total(0),
		connections_current(0),
		request_sent(0),
		response_received(0),
		error_occurred(0),
		groups_() {
		static thread_local std::size_t ClientId = 0;
		std::vector<seastar::metrics::label_instance> labels;
		labels.emplace_back("service", joinString("", "cpv-http-client-", ClientId++));
		labels.emplace_back("host-header", hostHeader);
		groups_.add_group("cpv-http-client", {
			seastar::metrics::make_derive(
				"connections_total",
				[this] { return connections_total; },
				seastar::metrics::description("The total number of connection opened"),
				labels),
			seastar::metrics::make_gauge(
				"connections_current",
				[this] { return connections_current; },
				seastar::metrics::description("The current number of open connections (in pools)"),
				labels),
			seastar::metrics::make_derive(
				"request_sent",
				[this] { return request_sent; },
				seastar::metrics::description("The total number of request sent"),
				labels),
			seastar::metrics::make_derive(
				"response_received",
				[this] { return response_received; },
				seastar::metrics::description("The total number of response received "),
				labels),
			seastar::metrics::make_derive(
				"error_occurred",
				[this] { return error_occurred; },
				seastar::metrics::description("The total number of error occurred"),
				labels)
		});
	}
}

