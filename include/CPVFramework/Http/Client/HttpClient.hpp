#pragma once
#include <string>
#include <string_view>
#include <core/future.hh>
#include <core/shared_ptr.hh>

namespace cpv {
	class HttpClientData;
	class HttpClientRequest;
	class HttpClientResponse;

	/**
	 * Used to send http requests to external host and receive response.
	 * This class manage multiple connections and can be used as singleton per core.
	 */
	class HttpClient {
	public:
		/** Send a http request by it's full conetent */
		seastar::future<HttpClientResponse> send(const std::string_view& str);

		/** make a http request */
		HttpClientRequest makeRequest(
			const std::string_view& method,
			const std::string_view& path);

		/** Create a plain http client */
		static HttpClient create(
			const std::string_view& hostname,
			std::uint16_t port);

		/** Create a ssl encrypted https client */
		static HttpClient createSSL(
			const std::string_view& hostname,
			std::uint16_t port,
			const std::string_view& x509Cert,
			const std::string_view& x509Key);

	private:
		/** Constructor */
		HttpClient();

	private:
		seastar::shared_ptr<HttpClientData> data_;
	};
}

