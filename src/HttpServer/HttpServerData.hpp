#pragma once
#include <seastar/core/shared_future.hh>
#include <seastar/net/api.hh>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/HttpServerRequestHandler.hpp>
#include "HttpServerMetricsData.hpp"

namespace cpv {
	/** Members of HttpServer */
	class HttpServerData {
	public:
		/** Constructor */
		HttpServerData(
			const HttpServerConfiguration& configurationVal,
			const std::vector<std::unique_ptr<HttpServerRequestHandler>>& handlersVal);
		
	public:
		seastar::lw_shared_ptr<const HttpServerConfiguration> configuration;
		seastar::lw_shared_ptr<const std::vector<std::unique_ptr<HttpServerRequestHandler>>> handlers;
		HttpServerMetricsData metricsData_;
		std::vector<seastar::server_socket> listeners;
		bool stopping;
		seastar::promise<> stoppedPromise;
		seastar::shared_future<> stoppedFuture;
	};
}

