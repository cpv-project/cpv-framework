#pragma once
#include <seastar/core/metrics_registration.hh>

namespace cpv {
	/** Define metrics targets for http server */
	class HttpServerMetricsData {
	public:
		/** Constructor */
		HttpServerMetricsData();
		
	public:
		/** The total number of connections accepted */
		std::uint64_t total_connections;
		/** The current number of open connections */
		std::uint64_t current_connections;
		/** The total number of http request served */
		std::uint64_t request_served;
		/** The total number of errors while reading from client */
		std::uint64_t read_errors;
		/** The total number of errors while writing to client */
		std::uint64_t respond_errors;
		
	private:
		seastar::metrics::metric_groups groups_;
	};
}

