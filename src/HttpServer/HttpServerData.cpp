#include "./HttpServerData.hpp"

namespace cpv {
	/** Constructor */
	HttpServerData::HttpServerData(
		const HttpServerConfiguration& configuration,
		const seastar::shared_ptr<Logger>& logger,
		std::vector<std::unique_ptr<HttpServerRequestHandlerBase>>&& handlers) :
		connectionsWrapper(seastar::make_lw_shared<HttpServerConnectionsWrapper>()),
		sharedData(seastar::make_lw_shared<HttpServerSharedData>(
			configuration, logger, std::move(handlers), connectionsWrapper->weak_from_this())),
		listeners(),
		listenerStoppedFutures(),
		stopping(false) { }
}

