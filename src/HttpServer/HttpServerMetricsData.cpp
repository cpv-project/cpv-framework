#include <seastar/core/metrics.hh>
#include <CPVFramework/Utility/StringUtils.hpp>
#include "./HttpServerMetricsData.hpp"

namespace cpv {
	/** Constructor */
	HttpServerMetricsData::HttpServerMetricsData() :
		total_connections(),
		current_connections(),
		request_served(),
		read_errors(),
		write_errors(),
		groups_() {
		static thread_local std::size_t ServiceId = 0;
		std::vector<seastar::metrics::label_instance> labels;
		labels.emplace_back("service", joinString("", "cpv-http-server-", ServiceId++));
		groups_.add_group("cpv-http-server", {
			seastar::metrics::make_derive(
				"total_connections",
				[this] { return total_connections; },
				seastar::metrics::description("The total number of connections accepted"),
				labels),
			seastar::metrics::make_gauge(
				"current_connections",
				[this] { return current_connections; },
				seastar::metrics::description("The current number of open connections"),
				labels),
			seastar::metrics::make_derive(
				"request_served",
				[this] { return request_served; },
				seastar::metrics::description("The total number of http request served"),
				labels),
			seastar::metrics::make_derive(
				"read_errors",
				[this] { return read_errors; },
				seastar::metrics::description("The total number of errors while reading from client"),
				labels),
			seastar::metrics::make_derive(
				"write_errors",
				[this] { return write_errors; },
				seastar::metrics::description("The total number of errors while writing to client"),
				labels)
		});
	}
}

