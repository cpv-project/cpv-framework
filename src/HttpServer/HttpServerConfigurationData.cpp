#include "./HttpServerConfigurationData.hpp"

namespace cpv {
	static const std::size_t DefaultMaxInitialRequestBytes = 524288;
	static const std::size_t DefaultMaxInitialRequestPackets = 512;
	
	/** Constructor */
	HttpServerConfigurationData::HttpServerConfigurationData() :
		listenAddresses(),
		maxInitialRequestBytes(DefaultMaxInitialRequestBytes),
		maxInitialRequestPackets(DefaultMaxInitialRequestPackets) { }
}

