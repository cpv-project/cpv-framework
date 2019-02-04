#pragma once
#include <memory>
#include <vector>
#include <seastar/core/future.hh>
#include "../Logging/Logger.hpp"
#include "./HttpServerConfiguration.hpp"
#include "./Handlers/HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Members of HttpServer */
	class HttpServerData;
	
	/**
	 * Http server object used to accept and handle http connections,
	 * Should run on each core.
	 */
	class HttpServer {
	public:
		/** Start accept http connections */
		seastar::future<> start();
		
		/** Stop accept http connection and close all exists connections  */
		seastar::future<> stop();
		
		/**
		 * Constructor.
		 * Rules about handler list:
		 *	The first handler should be a 500 handler. (e.g. HttpServerRequest500Handler)
		 *	The last handler should be a 404 handler. (e.g. HttpServerRequest404Handler)
		 *	The last handler must not call the next handler, there is a real last handler
		 *		but only returns exception future.
		 */
		HttpServer(
			const HttpServerConfiguration& configuration,
			const seastar::shared_ptr<Logger>& logger,
			std::vector<std::unique_ptr<HttpServerRequestHandlerBase>>&& handlers);
		
		/** Move constructor (for incomplete member type) */
		HttpServer(HttpServer&&);
		
		/** Destructor (for incomplete member type) */
		~HttpServer();
		
	private:
		std::unique_ptr<HttpServerData> data_;
	};
}

