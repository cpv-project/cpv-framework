#pragma once
#include <memory>
#include <vector>
#include <seastar/core/future.hh>
#include "HttpServerConfiguration.hpp"
#include "HttpServerRequestHandler.hpp"

namespace cpv {
	/** Defines members of HttpServer */
	class HttpServerData;
	
	/**
	 * Http server object used to accept and handle http connections,
	 * Should run on each core.
	 */
	class HttpServer {
	public:
		/**
		 * Constructor.
		 * Rules about handler list:
		 *	The first handler should be an error handler. (e.g. HttpServerRequestErrorHandler)
		 *	The last handler should be a 404 handler. (e.g. HttpServerRequest404Handler)
		 *	The last handler must not access the next handler because it's invalid pointer.
		 */
		HttpServer(
			const HttpServerConfiguration& configuration,
			const std::vector<std::unique_ptr<HttpServerRequestHandler>>& handlers);
		
		/** Start the processing loop */
		seastar::future<> start();
		
		/** Make a started http server quit it's loop */
		seastar::future<> stop();
		
	private:
		std::unique_ptr<HttpServerData> data_;
	};
}

