#pragma once
#include <unordered_set>
#include <seastar/core/weak_ptr.hh>
#include <seastar/core/metrics_registration.hh>
#include <CPVFramework/Container/Container.hpp>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp>
#include <CPVFramework/Logging/Logger.hpp>
#include "./Connections/HttpServerConnectionBase.hpp"

namespace cpv {
	/** Wrap http server connections to make it support weak reference */
	class HttpServerConnectionsWrapper :
		public seastar::weakly_referencable<HttpServerConnectionsWrapper> {
	public:
		/** Constructor */
		HttpServerConnectionsWrapper();
		
	public:
		/** Store all active connections, connections will remove self from it when close */
		std::unordered_set<seastar::shared_ptr<HttpServerConnectionBase>> value;
	};
	
	/** Data shared between HttpServer and HttpServerConnections */
	class HttpServerSharedData {
	public:
		/** Constructor */
		HttpServerSharedData(
			const Container& containerVal,
			seastar::weak_ptr<HttpServerConnectionsWrapper>&& connectionsWrapperVal);
		
	public:
		const Container container;
		const HttpServerConfiguration configuration;
		const seastar::shared_ptr<Logger> logger;
		const HttpServerRequestHandlerCollection handlers;
		const seastar::weak_ptr<HttpServerConnectionsWrapper> connectionsWrapper;
		
	public:
		/** Metric targets */
		struct {
			/** The total number of connections accepted */
			std::uint64_t total_connections = 0;
			/** The current number of open connections */
			std::uint64_t current_connections = 0;
			/** The total number of http request received */
			std::uint64_t request_received = 0;
			/** The total number of errors while handling http request */
			std::uint64_t request_errors = 0;
			/** The total number of timeout errors while handling http request */
			std::uint64_t request_timeout_errors = 0;
			/** The total number of initial size errors while handling http request */
			std::uint64_t request_initial_size_errors = 0;
			/** The total number of invalid format errors while handling http request */
			std::uint64_t request_invalid_format_errors = 0;
		} metricData;
		
	private:
		seastar::metrics::metric_groups metricGroups_;
	};
}

