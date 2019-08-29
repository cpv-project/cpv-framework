#include "./HttpServerData.hpp"

namespace cpv {
	/** Constructor */
	HttpServerData::HttpServerData(const Container& container) :
		connectionsWrapper(seastar::make_lw_shared<HttpServerConnectionsWrapper>()),
		sharedData(seastar::make_lw_shared<HttpServerSharedData>(
			container, connectionsWrapper->weak_from_this())),
		listeners(),
		listenerStoppedFutures(),
		detectTimeoutTimer(),
		stopping(false) { }
}

