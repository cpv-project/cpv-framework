#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>

namespace cpv {
	/** Get the configuration */
	HttpServerConfiguration& HttpServerModule::getConfig() & {
		return config_;
	}

	/** Set the request handler for page not found (usually be the last handler) */
	void HttpServerModule::set404Handler(
		const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler) {
		http404Handler_ = handler;
	}

	/** Set the request handler for exception (usually be the first handler) */
	void HttpServerModule::set500Handler(
		const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler) {
		http500Handler_ = handler;
	}

	/** Add custom handler between 404 and 500 handler */
	void HttpServerModule::addCustomHandler(
		const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler) {
		customHandlers_.emplace_back(handler);
	}

	/** Do some work for given application state */
	seastar::future<> HttpServerModule::handle(
		Container& container, ApplicationState state) {
		if (state == ApplicationState::RegisterHeadServices) {
			if (http500Handler_) {
				container.add(http500Handler_);
			} else {
				container.add<
					seastar::shared_ptr<HttpServerRequestHandlerBase>,
					seastar::shared_ptr<HttpServerRequest500Handler>>(
					ServiceLifetime::Persistent);
			}
		} else if (state == ApplicationState::RegisterServices) {
			container.add<HttpServerConfiguration>(config_);
			for (auto& customHandler : customHandlers_) {
				container.add(customHandler);
			}
		} else if (state == ApplicationState::RegisterTailServices) {
			if (http404Handler_) {
				container.add(http404Handler_);
			} else {
				container.add<
					seastar::shared_ptr<HttpServerRequestHandlerBase>,
					seastar::shared_ptr<HttpServerRequest404Handler>>(
					ServiceLifetime::Persistent);
			}
		} else if (state == ApplicationState::AfterServicesRegistered) {
			server_ = HttpServer(container);
		} else if (state == ApplicationState::Starting) {
			return server_.value().start();
		} else if (state == ApplicationState::Stopping) {
			return server_.value().stop();
		}
		return seastar::make_ready_future<>();
	}

	/** Constructor */
	HttpServerModule::HttpServerModule() :
		config_(),
		http404Handler_(),
		http500Handler_(),
		customHandlers_(),
		server_() {
		config_.setListenAddresses({ "127.0.0.1:8000" });
	}
}

