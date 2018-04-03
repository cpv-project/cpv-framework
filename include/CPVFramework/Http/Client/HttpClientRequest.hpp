#pragma once
#include <string>
#include <string_view>
#include <core/future.hh>

namespace cpv {
	class HttpClient;
	class HttpClientResponse;

	/** Http request builder */
	class HttpClientRequest {
	public:
		/** Add a header */
		HttpClientRequest& addHeader(
			const std::string_view& key,
			const std::string_view& value) &;

		/** Set the body of this request */
		HttpClientRequest& setBody(
			const std::string_view& mimeType,
			const std::string_view& content) &;

		/** Send this http request */
		seastar::future<HttpClientResponse> send(HttpClient& client);

		/** Get the full content of this request */
		const std::string& str() const & { return str_; }
		std::string& str() & { return str_; }

	private:
		/** Constructor */
		HttpClientRequest(
			const std::string_view& method,
			const std::string_view& path,
			const std::string_view& host);

	private:
		bool headerFinished_;
		std::string str_;
		friend HttpClient;
	};
}

