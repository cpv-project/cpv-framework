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
		std::chrono::milliseconds requestTimeout;
		std::size_t requestQueueSize;
		std::size_t requestBodyQueueSize;
		
		/** Constructor */
		HttpServerConfigurationData();
	};
}

