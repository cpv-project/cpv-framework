#pragma once
#include <string>
#include <vector>
#include <memory>

namespace cpv {
	/** Members of HttpServerConfiguration */
	class HttpServerConfigurationData;
	
	/** Contains all configuration items of a http server */
	class HttpServerConfiguration {
	public:
		/** Get listen addresses, e.g. "0.0.0.0:80" */
		const std::vector<std::string>& getListenAddresses() const&;
		
		/** Set listen address */
		void setListenAddresses(std::vector<std::string>&& listenAddresses);
		
		/** Constructor */
		HttpServerConfiguration();
		
		/** Destructor */
		~HttpServerConfiguration();
		
		/** Copy and move */
		HttpServerConfiguration(const HttpServerConfiguration& other);
		HttpServerConfiguration(HttpServerConfiguration&& other);
		HttpServerConfiguration& operator=(const HttpServerConfiguration& other);
		HttpServerConfiguration& operator=(HttpServerConfiguration&& other);
		
	private:
		std::unique_ptr<HttpServerConfigurationData> data_;
	};
	
	/** TODO: support json serialize and deserialize */
}

