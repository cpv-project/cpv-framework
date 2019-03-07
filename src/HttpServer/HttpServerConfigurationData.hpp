#pragma once
#include <chrono>
#include <string>
#include <vector>

namespace cpv {
	/** Members of HttpServerConfiguration */
	class HttpServerConfigurationData {
	public:
		std::vector<std::string> listenAddresses;
		std::size_t maxInitialRequestBytes;
		std::size_t maxInitialRequestPackets;
		std::chrono::milliseconds initialRequestTimeout;
		
		/** Constructor */
		HttpServerConfigurationData();
	};
}

