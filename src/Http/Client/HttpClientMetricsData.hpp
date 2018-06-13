#pragma once
#include <core/metrics_registration.hh>

namespace cpv {
	/** Define metrics targets for http client */
	class HttpClientMetricsData {
	public:
		/** Constructor */
		explicit HttpClientMetricsData(const std::string& hostHeader);

	public:
		/** The total number of connection opened */
		std::size_t connections_total;
		/** The current number of open connections (in pools) */
		std::size_t connections_current;
		/** The total number of request sent */
		std::size_t request_sent;
		/** The total number of response received */
		std::size_t response_received;
		/** The total number of error occurred */
		std::size_t error_occurred;

	private:
		seastar::metrics::metric_groups groups_;
	};
}

