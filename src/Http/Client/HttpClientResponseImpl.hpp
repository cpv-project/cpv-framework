#pragma once
#include <CPVFramework/Http/Client/HttpClientResponse.hpp>

namespace cpv {
	/** Implementation of http response content */
	class HttpClientResponseImpl : public HttpClientResponse {
	public:
		/** For Object */
		void reset();

		/** For Object */
		static void freeResources();

		/** Get the http status code */
		std::size_t getStatus() const override;

		/** Get all headers */
		const std::unordered_map<std::string_view, std::string_view>& getHeaders() const& override;

		/** Get a single header value by key, return empty if not exist */
		const std::string_view getHeader(const std::string_view& key) const& override;

		/** Get the response body */
		const std::string_view getBody() const& override;

		/** Append received data, return whether is all content received */
		bool appendReceived(const std::string_view& buf);
 
		/** Constructor */
		HttpClientResponseImpl();

	private:
		enum class State { Initial = 0, HeaderFinished = 1 };
		State state_;
		std::size_t status_;
		std::size_t contentLength_;
		std::unordered_map<std::string_view, std::string_view> headers_;
		std::string headerStr_;
		std::string bodyStr_;
	};
}

