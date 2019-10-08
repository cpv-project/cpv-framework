#pragma once
#include "../../HttpServer/Handlers/HttpServerRequestRoutingHandler.hpp"
#include "../../HttpServer/Handlers/HttpServerRequestFunctionHandler.hpp"
#include "../ModuleBase.hpp"

namespace cpv {
	/**
	 * Module used to add request routing handler for http server
	 * 
	 * You can associate handler with given path in the custom initialize function:
	 * ```
	 * application.add<HttpServerRoutingModule>(auto& module) {
	 *     module.route(cpv::constants::GET, "/", [] (HttpContext& context) {
	 *         return extensions::reply(context.getResponse(), "index page");
	 *     });
	 *     // pass a tuple to route function can make it retrive parameters automatically.
	 *     // integer means index of path fragments: 1 => request.getUri().getPathFragment(1)
	 *     // string means key of query parameters: "key" => request.getUri().getQueryParameter("key")
	 *     // you can make it support more types by provide cpv::extensions::getParameter(request, yourType).
	 *     // please remove space between / and * (comment syntax problem).
	 *     module.route(cpv::constants::GET, "/get/ *", std::make_tuple(1),
	 *         [] (HttpContext& context, std::string_view id) {
	 *             return extensions::reply(context.getResponse(), id);
	 *         });
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

		/** Do some work for given application state */
		seastar::future<> handle(Container& container, ApplicationState state) override;

		/** Constructor */
		HttpServerRoutingModule();

    private:
		seastar::shared_ptr<HttpServerRequestRoutingHandler> routingHandler_;
    };
}

