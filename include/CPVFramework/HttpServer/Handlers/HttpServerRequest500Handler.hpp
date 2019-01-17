#pragma once
#include "../../Logging/Logger.hpp"
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/**
	 * Request handler that capture exceptions from next handlers and return 500.
	 * The exception will log to given logger with a unique uuid,
	 * and the uuid will display on the error page for finding associated error log.
	 */
	class HttpServerRequest500Handler : public HttpServerRequestHandlerBase {
	public:
		/** Return 500 internal server error */
		seastar::future<> handle(
			HttpRequest& request,
			HttpResponse& response,
			const HttpServerRequestHandlerIterator& next) const override;
		
		/** Constructor */
		explicit HttpServerRequest500Handler(const seastar::shared_ptr<cpv::Logger> logger);
		
	private:
		seastar::shared_ptr<cpv::Logger> logger_;
	};
}

