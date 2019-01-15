#pragma once
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Request handler that returns 404 not found as page content (no redirect) */
	class HttpServerRequest404Handler : public HttpServerRequestHandlerBase {
	public:
		/** Return 404 not found */
		seastar::future<> handle(
			HttpRequest&,
			HttpResponse& response,
			HttpServerRequestHandlerIterator) const override;
	};
}

