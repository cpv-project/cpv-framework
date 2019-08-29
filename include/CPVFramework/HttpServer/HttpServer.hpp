#pragma once
#include <memory>
#include <vector>
#include <seastar/core/future.hh>
#include "../Container/Container.hpp"
#include "../Logging/Logger.hpp"
#include "./Handlers/HttpServerRequestHandlerBase.hpp"
#include "./HttpServerConfiguration.hpp"

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
		 * Constructor
		 *
		 * Container should contains following services:
		 * - HttpServerConfiguration
		 * - seastar::shared_ptr<Logger>
		 * - one or more std::shared_ptr<HttpServerRequestHandlerBase>, order is matter
		 *
		 * Rules about handler list:
		 *	The first handler should be a 500 handler. (e.g. HttpServerRequest500Handler)
		 *	The last handler should be a 404 handler. (e.g. HttpServerRequest404Handler)
		 *	The last handler must not call the next handler, there is a real last handler
		 *		but only returns exception future.
		 */
		HttpServer(const Container& container);
		
		/** Move constructor (for incomplete member type) */
		HttpServer(HttpServer&&);
		
		/** Destructor (for incomplete member type) */
		~HttpServer();
		
	private:
		std::unique_ptr<HttpServerData> data_;
	};
}

