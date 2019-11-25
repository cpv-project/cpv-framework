#pragma once
#include "../../Utility/SharedString.hpp"
#include "../../Utility/Uri.hpp"
#include "./HttpServerRequestHandlerBase.hpp"
#include "./HttpServerRequestFunctionHandler.hpp"
#include "./HttpServerRequestParametersFunctionHandler.hpp"

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
		 * Associate handler with given method and path
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
		void route(SharedString&& method, SharedString&& path,
			const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler);

		/** Associate handler function with given method and path */
		template <class Func, std::enable_if_t<
			std::is_base_of_v<
				HttpServerRequestHandlerBase,
				HttpServerRequestFunctionHandler<Func>>, int> = 0>
		void route(SharedString&& method, SharedString&& path, Func func) {
			return route(std::move(method), std::move(path), seastar::make_shared<
				HttpServerRequestFunctionHandler<Func>>(std::move(func)));
		}

		/** Associate handler function that takes given parameters with given method and path */
		template <class Params, class Func, std::enable_if_t<
			std::is_base_of_v<
				HttpServerRequestHandlerBase,
				HttpServerRequestParametersFunctionHandler<Params, Func>>, int> = 0>
		void route(SharedString&& method, SharedString&& path, Params params, Func func) {
			return route(std::move(method), std::move(path), seastar::make_shared<
				HttpServerRequestParametersFunctionHandler<Params, Func>>(
				std::move(params), std::move(func)));
		}

		/** Remove associated handler with given method and path */
		void removeRoute(const SharedString& method, const SharedString& path);

		/** Get associated handler with given method and uri, return nullptr if not found */
		seastar::shared_ptr<HttpServerRequestHandlerBase> getRoute(
			const SharedString& method, const Uri& uri) const;

		/** Get associated handler with given method and path, return nullptr if not found */
		seastar::shared_ptr<HttpServerRequestHandlerBase> getRoute(
			const SharedString& method, const SharedString& path) const;

		/** Handle request depends on the routing table */
		seastar::future<> handle(
			HttpContext& context,
			HttpServerRequestHandlerIterator next) const override;

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

