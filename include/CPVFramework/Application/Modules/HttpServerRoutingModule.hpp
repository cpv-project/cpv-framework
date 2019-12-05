#pragma once
#include "../../HttpServer/Handlers/HttpServerRequestRoutingHandler.hpp"
#include "../../HttpServer/Handlers/HttpServerRequestFunctionHandler.hpp"
#include "../../HttpServer/Handlers/HttpServerRequestStaticFileHandler.hpp"
#include "../ModuleBase.hpp"

namespace cpv {
	/**
	 * Module used to add request routing handler for http server
	 * 
	 * You can associate handler with given path in the custom initialize function:
	 * ```
	 * application.add<HttpServerRoutingModule>(auto& module) {
	 *     module.route(constants::GET, "/", [] (HttpContext& context) {
	 *         return extensions::reply(context.getResponse(), "index page");
	 *     });
	 *     // pass a tuple to route function can make it retrive parameters automatically
	 *     // for more information please see:
	 *     // ../HttpServer/Handlers/HttpServerRequestParametersFunctionHandler.hpp
	 *     // please remove space between / and * (comment syntax problem)
	 *     using namespace cpv::extensions::http_context_parameters;
	 *     module.route(constants::GET, "/get/ *",
	 *         std::make_tuple(PathFragment(1), Query("abc")),
	 *         [] (HttpContext& context, SharedString id, SharedString name) {
	 *         Packet p;
	 *         p.append(std::move(id)).append(": ").append(std::move(name));
	 *         return extensions::reply(context.getResponse(), std::move(p));
	 *     });
	 * });
	 * ```
	 * 
	 * In addition, this module will register seastar::shared_ptr<HttpServerRequestRoutingHandler>
	 * with persistent lifetime to the container, other modules can get the routing handler from the
	 * container and use it to associate more custom handlers to custom pathes.
	 *
	 * Notice:
	 * Since the module order is matter, the position of routing handler in the handler list
	 * depends on the module order, the routing handler will be registered at RegisterServices state.
	 */
	class HttpServerRoutingModule : public ModuleBase {
	public:
		/**
		 * Associate handler with given path
		 * For rules please see comments in HttpServerRequestRoutingHandler::route
		 */
		template <class... Args>
		void route(Args&&... args) {
			routingHandler_->route(std::forward<Args>(args)...);
		}

		/**
		 * Associate static file handler with given path
		 * For parameters please see the constructor of HttpServerRequestStaticFileHandler
		 */
		template <class... Args>
		void routeStaticFile(SharedString&& urlBase, Args&&... args) {
			SharedString path(SharedStringBuilder()
				.append(trimString<false, true>(urlBase, '/'))
				.append("/**")
				.build());
			routingHandler_->route(
				constants::GET, std::move(path),
				seastar::make_shared<HttpServerRequestStaticFileHandler>(
					std::move(urlBase), std::forward<Args>(args)...));
		}

		/** Do some work for given application state */
		seastar::future<> handle(Container& container, ApplicationState state) override;

		/** Constructor */
		HttpServerRoutingModule();

    private:
		seastar::shared_ptr<HttpServerRequestRoutingHandler> routingHandler_;
    };
}

