#pragma once
#include "../../Logging/Logger.hpp"
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/**
	 * Request handler that capture exceptions from next handlers and return 500.
	 * The exception will associate with an unique uuid and log with 'Error' level,
	 * and the uuid will display inside the response body.
	 */
	class HttpServerRequest500Handler : public HttpServerRequestHandlerBase {
	public:
		using DependencyTypes = std::tuple<seastar::shared_ptr<Logger>>;

		/** Return 500 internal server error */
		seastar::future<> handle(
			HttpContext& context,
			const HttpServerRequestHandlerIterator& next) const override;

		/** Constructor */
		explicit HttpServerRequest500Handler(seastar::shared_ptr<Logger> logger);

	private:
		seastar::shared_ptr<Logger> logger_;
	};
}

