#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

namespace cpv {
	class HttpClient;

	/** Http response content */
	class HttpClientResponse {
	public:
		/** Get the http status code */
		std::size_t getStatus() const { return status_; }

		/** Get all headers */
		const std::unordered_map<std::string_view, std::string_view>& getHeaders() const& { return headers_; }

		/** Get a single header value by key, return empty if not exist */
		const std::string_view getHeader(const std::string_view& key) const&;

		/** Get the response body */
		const std::string& getBody() const& { return bodyStr_; }

		/** Append received data, return whether is all content received */
		bool appendReceived(const std::string_view& buf);

		/** Constructor */
		HttpClientResponse();

	private:
		enum class State { Initial = 0, HeaderFinished = 1 };
		State state_;
		std::size_t status_;
		std::size_t contentLength_;
		std::unordered_map<std::string_view, std::string_view> headers_;
		std::string headerStr_;
		std::string bodyStr_;
		friend HttpClient;
	};
}

