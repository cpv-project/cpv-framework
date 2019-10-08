#include <CPVFramework/Application/Modules/HttpServerRoutingModule.hpp>

namespace cpv {
	/** Do some work for given application state */
	seastar::future<> HttpServerRoutingModule::handle(Container& container, ApplicationState state) {
		if (state == ApplicationState::RegisterServices) {
			container.add<seastar::shared_ptr<HttpServerRequestHandlerBase>>(routingHandler_);
			container.add<seastar::shared_ptr<HttpServerRequestRoutingHandler>>(routingHandler_);
		}
		return seastar::make_ready_future<>();
	}

	/** Constructor */
	HttpServerRoutingModule::HttpServerRoutingModule() :
		routingHandler_(seastar::make_shared<HttpServerRequestRoutingHandler>()) { }
}

