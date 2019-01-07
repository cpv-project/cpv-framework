#pragma once
#include <seastar/core/future.hh>

namespace cpv {
	/** The interface of a http server request handler */
	class HttpServerRequestHandler {
	public:
		/** Virtual destructor */
		virtual ~HttpServerRequestHandler() = default;
		
		/** Constructor */
		virtual seastar::future<> handle(
			HttpServerRequest& request,
			HttpServerResponse& response) = 0;
	};
}

