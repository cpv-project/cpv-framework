#pragma once
#include "HttpClient.hpp"

namespace cpv {
	/**
	 * Interface of http client factory.
	 * Should return same instance of same hostname and port.
	 */
	class HttpClientFactory {
	public:
		/** Create a plain http client */
		virtual seastar::shared_ptr<HttpClient> create(
			const std::string_view& hostname,
			std::uint16_t port) = 0;

		/** Create a ssl encrypted https client */
		virtual seastar::shared_ptr<HttpClient> createSSL(
			const std::string_view& hostname,
			std::uint16_t port,
			const std::string_view& x509Cert,
			const std::string_view& x509Key) = 0;
	};
}

