#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

namespace cpv {
	/** Interface of http response content */
	class HttpClientResponse {
	public:
		/** Get the http status code */
		virtual std::size_t getStatus() const = 0;

		/** Get all headers */
		virtual const std::unordered_map<std::string_view, std::string_view>& getHeaders() const& = 0;

		/** Get a single header value by key, return empty if not exist */
		virtual const std::string_view getHeader(const std::string_view& key) const& = 0;

		/** Get the response body */
		virtual const std::string_view getBody() const& = 0;
	};
}

