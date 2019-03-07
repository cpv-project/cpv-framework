#include "./HttpServerConfigurationData.hpp"

namespace cpv {
	static const std::size_t DefaultMaxInitialRequestBytes = 524288;
	static const std::size_t DefaultMaxInitialRequestPackets = 512;
	static const std::size_t DefaultInitialRequestTimeout = 30000;
	
	/** Constructor */
	HttpServerConfigurationData::HttpServerConfigurationData() :
		listenAddresses(),
		maxInitialRequestBytes(DefaultMaxInitialRequestBytes),
		maxInitialRequestPackets(DefaultMaxInitialRequestPackets),
		initialRequestTimeout(DefaultInitialRequestTimeout) { }
}

