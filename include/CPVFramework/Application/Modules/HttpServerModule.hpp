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
	 * application.add<LoggingModule>();
	 * application.add<HttpServerModule>(auto& module) {
	 *     auto& config = module.getConfig();
	 *     config.setListenAddresses({ "0.0.0.0:80" }); // listen on 80 for all hosts
	 *     module.set404Handler([] (HttpContext& context) {
	 *         // redirect to /404 if path not found
	 *         return extensions::redirectTo(context.getResponse(), "/404");
	 *     });
	 *     // unlike 400 handler, 500 handler should only act when exception occurs,
	 *     // you could see src/HttpServer/Handlers/HttpServerRequest500Handler.cpp
	 *     // if you want to write a custom 500 handler.
	 *     module.set500Handler(seastar::make_shared<My500Handler>());
	 *     // custom handler is for simple application or testing purpose
	 *     // usually you should use HttpServerRoutingModule for most cases,
	 *     // or add a new module to register handles to container
	 *     module.addCustomHandler(seastar::make_shared<MyHandler>());
	 * });
	 * ```
	 *
	 * This module will make the http server start when application start,
	 * and make the http server stop when application stop, on each cpu cores,
	 * no addition code are required.
	 *
	 * Notice:
	 * This module require seastar::shared_ptr<Logger> registered in container,
	 * please also add LoggingModule to application.
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
		void set404Handler(Func func) {
			set404Handler(seastar::make_shared<
				HttpServerRequestFunctionHandler<Func>>(std::move(func)));
		}

		/** Set the request handler for exception (usually be the first handler) */
		void set500Handler(const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler);

		/** Add custom handler between 404 and 500 handler */
		void addCustomHandler(const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler);

		/** Add custom handler between 404 and 500 handler */
		template <class Func, std::enable_if_t<
			std::is_base_of_v<
				HttpServerRequestHandlerBase,
				HttpServerRequestFunctionHandler<Func>>, int> = 0>
		void addCustomHandler(Func func) {
			addCustomHandler(seastar::make_shared<
				HttpServerRequestFunctionHandler<Func>>(std::move(func)));
		}

		/** Do some work for given application state */
		seastar::future<> handle(Container& container, ApplicationState state) override;

		/** Constructor */
		HttpServerModule();

	private:
		HttpServerConfiguration config_;
		seastar::shared_ptr<HttpServerRequestHandlerBase> http404Handler_;
		seastar::shared_ptr<HttpServerRequestHandlerBase> http500Handler_;
		HttpServerRequestHandlerCollection customHandlers_;
		std::optional<HttpServer> server_; // lazy initialized
	};
}

