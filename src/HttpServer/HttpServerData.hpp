#pragma once
#include <utility>
#include <seastar/core/shared_future.hh>
#include <seastar/net/api.hh>
#include "./HttpServerSharedData.hpp"

namespace cpv {
	/** Declare types */
	class HttpServerConnectionBase;
	
	/** Members of HttpServer */
	class HttpServerData {
	public:
		/** Constructor */
		HttpServerData(
			const HttpServerConfiguration& configuration,
			const seastar::shared_ptr<Logger>& logger,
			HttpServerRequestHandlerCollection&& handlers);
		
	public:
		seastar::lw_shared_ptr<HttpServerConnectionsWrapper> connectionsWrapper;
		seastar::lw_shared_ptr<HttpServerSharedData> sharedData;
		std::vector<seastar::lw_shared_ptr<
			std::pair<seastar::server_socket, seastar::socket_address>>> listeners;
		std::vector<seastar::future<>> listenerStoppedFutures;
		bool stopping;
	};
}

