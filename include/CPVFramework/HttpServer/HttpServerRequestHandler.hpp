#pragma once
#include <vector>
#include <memory>
#include <seastar/core/future.hh>
#include "../Http/HttpRequest.hpp"
#include "../Http/HttpResponse.hpp"

namespace cpv {
	/** The iterator type of HttpServerRequestHandler */
	class HttpServerRequestHandler;
	using HttpServerRequestHandlerIterator =
		std::vector<std::unique_ptr<HttpServerRequestHandler>>::const_iterator;
	
	/** The interface of a http server request handler */
	class HttpServerRequestHandler {
	public:
		/** Virtual destructor */
		virtual ~HttpServerRequestHandler() = default;
		
		/**
		 * Handle a http server request.
		 * If the request is not handled in this handler, pass it to the next handler:
		 *	(*next)->handle(request, response, next + 1);
		 */
		virtual seastar::future<> handle(
			HttpRequest& request,
			HttpResponse& response,
			HttpServerRequestHandlerIterator next) const = 0;
	};
}

