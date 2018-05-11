#pragma once
#include <string>
#include <string_view>
#include <core/future.hh>
#include <core/shared_ptr.hh>
#include "../../Utility/Object.hpp"

namespace cpv {
	class HttpClientRequest;
	class HttpClientResponse;

	/**
	 * Interface of http client.
	 * This class manage multiple connections and can be used as singleton per core.
	 */
	class HttpClient {
	public:
		/** Return whether it's a https client */
		virtual bool isSSL() const = 0;

		/** make a http request */
		virtual Object<HttpClientRequest> makeRequest(
			const std::string_view& method, const std::string_view& path) = 0;

		/** Send a http request */
		virtual seastar::future<Object<HttpClientResponse>> send(Object<HttpClientRequest>&& request) = 0;
	};
}

