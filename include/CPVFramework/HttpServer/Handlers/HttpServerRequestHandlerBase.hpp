#pragma once
#include <vector>
#include <seastar/core/shared_ptr.hh>
#include <seastar/core/future.hh>
#include "../HttpContext.hpp"

namespace cpv {
	/** The iterator type of HttpServerRequestHandler */
	class HttpServerRequestHandlerBase;
	using HttpServerRequestHandlerCollection = std::vector<seastar::shared_ptr<HttpServerRequestHandlerBase>>;
	using HttpServerRequestHandlerIterator = HttpServerRequestHandlerCollection::const_iterator;
	
	/** The interface of a http server request handler */
	class HttpServerRequestHandlerBase {
	public:
		/** Virtual destructor */
		virtual ~HttpServerRequestHandlerBase() = default;
		
		/**
		 * Handle a http server request.
		 * If the request is not handled in this handler, pass it to the next handler:
		 *	(*next)->handle(context, next + 1);
		 */
		virtual seastar::future<> handle(
			HttpContext& context,
			const HttpServerRequestHandlerIterator& next) const = 0;
	};
}

