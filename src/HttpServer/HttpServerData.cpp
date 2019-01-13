#include "./HttpServerData.hpp"

namespace cpv {
	/** Constructor */
	HttpServerData::HttpServerData(
		const HttpServerConfiguration& configurationVal,
		const std::vector<seastar::shared_ptr<HttpServerRequestHandlerBase>>& handlersVal) :
		configuration(seastar::make_lw_shared<HttpServerConfiguration>(configurationVal)),
		handlers(),
		connections(seastar::make_lw_shared<
			std::unordered_set<seastar::shared_ptr<HttpServerConnectionBase>>>()),
		metricsData(),
		listeners(),
		listenerStoppedFutures(),
		stopping(false),
		stoppedPromise(),
		stoppedFuture(stoppedPromise.get_future()) {
		// TODO: add internal real last handler
		auto handlersCopy = handlersVal;
		handlers = seastar::make_lw_shared<
			std::vector<seastar::shared_ptr<HttpServerRequestHandlerBase>>>(handlersCopy);
	}
}

