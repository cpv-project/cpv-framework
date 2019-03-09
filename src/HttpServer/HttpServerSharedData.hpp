#pragma once
#include <unordered_set>
#include <seastar/core/weak_ptr.hh>
#include <seastar/core/metrics_registration.hh>
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
			const HttpServerConfiguration& configurationVal,
			const seastar::shared_ptr<Logger>& loggerVal,
			HttpServerRequestHandlerCollection&& handlersVal,
			seastar::weak_ptr<HttpServerConnectionsWrapper>&& connectionsWrapperVal);
		
	public:
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
			/** The total number of http request served */
			std::uint64_t request_served = 0;
			/** The total number of errors while reading from client */
			std::uint64_t read_errors = 0;
			/** The total number of errors while writing to client */
			std::uint64_t write_errors = 0;
		} metricData;
		
	private:
		seastar::metrics::metric_groups metricGroups_;
	};
}

