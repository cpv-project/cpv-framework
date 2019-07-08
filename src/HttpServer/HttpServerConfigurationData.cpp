#include "./HttpServerConfigurationData.hpp"

namespace cpv {
	static const std::size_t DefaultMaxInitialRequestBytes = 524288;
	static const std::size_t DefaultMaxInitialRequestPackets = 512;
	static const std::size_t DefaultRequestTimeout = 30000;
	static const std::size_t DefaultRequestQueueSize = 100;
	static const std::size_t DefaultRequestBodySize = 50;
	
	/** Constructor */
	HttpServerConfigurationData::HttpServerConfigurationData() :
		listenAddresses(),
		maxInitialRequestBytes(DefaultMaxInitialRequestBytes),
		maxInitialRequestPackets(DefaultMaxInitialRequestPackets),
		requestTimeout(DefaultRequestTimeout),
		requestQueueSize(DefaultRequestQueueSize),
		requestBodyQueueSize(DefaultRequestBodySize) { }
}

