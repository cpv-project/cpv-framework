#pragma once
#include <string>
#include <vector>

namespace cpv {
	/** Members of HttpServerConfiguration */
	class HttpServerConfigurationData {
	public:
		std::vector<std::string> listenAddresses;
		
		/** Constructor */
		HttpServerConfigurationData();
	};
}

