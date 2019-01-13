#pragma once
#include <unordered_set>
#include <seastar/core/shared_future.hh>
#include <seastar/net/api.hh>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp>
#include "./Connections/HttpServerConnectionBase.hpp"
#include "./HttpServerMetricsData.hpp"

namespace cpv {
	/** Members of HttpServer */
	class HttpServerData {
	public:
		/** Constructor */
		HttpServerData(
			const HttpServerConfiguration& configurationVal,
			const std::vector<seastar::shared_ptr<HttpServerRequestHandlerBase>>& handlersVal);
		
	public:
		seastar::lw_shared_ptr<const HttpServerConfiguration> configuration;
		seastar::lw_shared_ptr<const std::vector<
			seastar::shared_ptr<HttpServerRequestHandlerBase>>> handlers;
		seastar::lw_shared_ptr<std::unordered_set<
			seastar::shared_ptr<HttpServerConnectionBase>>> connections;
		HttpServerMetricsData metricsData;
		std::vector<seastar::server_socket> listeners;
		std::vector<seastar::future<>> listenerStoppedFutures;
		bool stopping;
		seastar::promise<> stoppedPromise;
		seastar::shared_future<> stoppedFuture;
	};
}

