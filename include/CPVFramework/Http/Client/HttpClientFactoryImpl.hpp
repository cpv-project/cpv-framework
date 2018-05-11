#pragma once
#include <unordered_map>
#include "HttpClientFactory.hpp"

namespace cpv {
	/**
	 * Implementation of http client factory.
	 * Should return same instance of same hostname and port.
	 */
	class HttpClientFactoryImpl : public HttpClientFactory {
	public:
		/** Create a plain http client */
		seastar::shared_ptr<HttpClient> create(
			const std::string_view& hostname,
			std::uint16_t port) override;

		/** Create a ssl encrypted https client */
		seastar::shared_ptr<HttpClient> createSSL(
			const std::string_view& hostname,
			std::uint16_t port,
			const std::string_view& x509Cert, // client certificate
			const std::string_view& x509Key) override;

		/** Constructor */
		HttpClientFactoryImpl();

	private:
		std::unordered_map<std::string, seastar::shared_ptr<HttpClient>> clients_;
	};
}

