#pragma once
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Members of HttpServerRequestRoutingHandler */
	class HttpServerRequestRoutingHandlerData;

	/**
	 * Request handler that use sub handler associated with request path to handle request,
	 * if no handler associated with request path it will invoke the next handler,
	 * sub handler also can invoke the next handler depends on their own logic.
	 */
	class HttpServerRequestRoutingHandler : public HttpServerRequestHandlerBase {
	public:
		/**
		 * Associate handler with given path
		 * Rules:
		 * - method and path is case sensitive
		 * - "*" matches a single path fragments
		 * - "**" matches multiple path fragments, only use it at the end of path
		 * Example (remove space around * because comment syntax doesn't allow / near *):
		 * - "/api/v1/users" matches "/api/v1/users"
		 * - "/api/v1/user/ *" matches "/api/v1/user/1" and "/api/v1/user/2"
		 * - "/api/v1/user/ * /logs" matches "/api/v1/user/1/logs" and "/api/v1/user/2/logs"
		 * - "/static/ **" matches "/static/js/1.js" and "/static/css/1.css"
		 */
		void route(std::string_view method, std::string_view path,
			const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler);

		/** Remove associated handler with given path */
		void removeRoute(std::string_view method, std::string_view path);

		/** Get associated handler with given path, return nullptr if not found */
		seastar::shared_ptr<HttpServerRequestHandlerBase> getRoute(
			std::string_view method, std::string_view path) const;

		/** Handle request depends on the routing table */
		seastar::future<> handle(
			HttpContext& context,
			const HttpServerRequestHandlerIterator& next) const override;

		/** Constructor */
		HttpServerRequestRoutingHandler();

		/** Move constructor (for incomplete member type) */
		HttpServerRequestRoutingHandler(HttpServerRequestRoutingHandler&&);

		/** Move assign operator (for incomplete member type) */
		HttpServerRequestRoutingHandler& operator=(HttpServerRequestRoutingHandler&&);

		/** Destructor (for incomplete member type) */
		~HttpServerRequestRoutingHandler();

	private:
		std::unique_ptr<HttpServerRequestRoutingHandlerData> data_;
	};
}

