#pragma once
#include <optional>
#include <seastar/core/shared_ptr.hh>
#include "../../HttpServer/HttpServer.hpp"
#include "../../HttpServer/Handlers/HttpServerRequestFunctionHandler.hpp"
#include "../ModuleBase.hpp"

namespace cpv {
	/**
	 * Module used to configure, start, and stop http server
	 * 
	 * By default, it listens on 127.0.0.1:8000 and use built-in handlers for 404 and 500,
	 * you can change the configuration by using custom initialize function:
	 * ```
	 * application.add<HttpServerModule>(auto& module) {
	 *     auto& config = module.getConfig();
	 *     config.setListenAddresses({ "0.0.0.0:80" }); // listen on 80 for all hosts
	 *     module.set404Handler([] (HttpContext& context) {
	 *         // TODO: implement this api
	 *         extensions::redirectTo(context.response, "/404");
	 *     });
	 *     // unlike 400 handler, 500 handler should only act when exception occurs,
	 *     // you could see src/HttpServer/Handlers/HttpServerRequest500Handler.cpp
	 *     // if you want to write a custom 500 handler.
	 *     module.set500Handler(seastar::make_shared<My500Handler>());
	 * });
	 * ```
	 *
	 * This module will make the http server start when application start,
	 * and make the http server stop when application stop, on each cpu cores,
	 * no addition code are required.
	 */
	class HttpServerModule : public ModuleBase {
	public:
		/** Get the configuration */
		HttpServerConfiguration& getConfig() &;

		/** Set the request handler for page not found (usually be the last handler) */
		void set404Handler(const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler);

		/** Set the request handler for page not found (usually be the last handler) */
		template <class Func, std::enable_if_t<
			std::is_base_of_v<
				HttpServerRequestHandlerBase,
				HttpServerRequestFunctionHandler<Func>>, int> = 0>
		void set404Handler(Func&& func) {
			set404Handler(seastar::make_shared<
				HttpServerRequestFunctionHandler>(std::forward<Func>(func)));
		}

		/** Set the request handler for exception (usually be the first handler) */
		void set500Handler(const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler);

		/** Do some work for given application state */
		seastar::future<> handle(Container& container, ApplicationState state) override;

		/** Constructor */
		HttpServerModule();

	private:
		HttpServerConfiguration config_;
		seastar::shared_ptr<HttpServerRequestHandlerBase> http404Handler_;
		seastar::shared_ptr<HttpServerRequestHandlerBase> http500Handler_;
		std::optional<HttpServer> server_; // lazy initialized
	};
}

