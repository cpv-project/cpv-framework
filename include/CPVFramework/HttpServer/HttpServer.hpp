#pragma once
#include <memory>
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
		/** Constructor */
		HttpServer(
			const HttpServerConfiguration& configuration,
			const std::vector<std::unique_ptr<HttpServerRequestHandler> handlers);
		
		/** Start the processing loop */
		seastar::future<> start();
		
		/** Make a started http server quit it's loop */
		void stop();
		
	private:
		std::unique_ptr<HttpServerData> data_;
	};
}

