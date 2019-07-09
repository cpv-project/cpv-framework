#pragma once
#include <chrono>
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
		
		/** Get bytes limitation of initial request data, the default value is 524288 (512KB). */
		std::size_t getMaxInitialRequestBytes() const;
		
		/**
		 * Set bytes limitation of initial request data, including headers and initial body.
		 * This is a security setting for protection of out of memory attack.
		 */
		void setMaxInitialRequestBytes(std::size_t maxInitialRequestBytes);
		
		/** Get packets limitation of initial request data, the default value is 512 */
		std::size_t getMaxInitialRequestPackets() const;
		
		/**
		 * Set packets limitation of initial request data,
		 * should be maxInitialRequestBytes / MTU + someSmallConstant.
		 * This is a security setting for protection of out of memory attack.
		 */
		void setMaxInitialRequestPackets(std::size_t maxInitialRequestPackets);
		
		/** Get timeout of request in milliseconds, the default value is 30000 (30s) */
		std::chrono::milliseconds getRequestTimeout() const;
		
		/**
		 * Set timeout of request in milliseconds.
		 * This setting will help http server close idle keep-alive connections.
		 * This setting will apply to every read operation of request (including body).
		 */
		void setRequestTimeout(const std::chrono::milliseconds& requestTimeout);
		
		/** Get the queue size of pending requests of single connection, the default value is 100 */
		std::size_t getRequestQueueSize() const;
		
		/** Set the queue size of pending requests of single connection */
		void setRequestQueueSize(std::size_t requestQueueSize);
		
		/** Get the queue size of pending body buffers of single connection, the default value is 50 */
		std::size_t getRequestBodyQueueSize() const;
		
		/** Set the queue size of pending body buffers of single connection */
		void setRequestBodyQueueSize(std::size_t requestBodyQueueSize);
		
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

