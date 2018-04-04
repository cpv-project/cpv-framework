#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Http/Client/HttpClientRequest.hpp>
#include <CPVFramework/Http/Client/HttpClient.hpp>
#include <CPVFramework/Http/Client/HttpClientResponse.hpp>

namespace cpv {
	/** Add a header */
	HttpClientRequest& HttpClientRequest::addHeader(
		const std::string_view& key,
		const std::string_view& value) {
		if (state_ != State::Initial) {
			throw LogicException(CPV_CODEINFO, "can't add header after body is set");
		}
		str_.append(key).append(": ").append(value).append("\r\n");
		return *this;
	}

	/** Set the body of this request */
	HttpClientRequest& HttpClientRequest::setBody(
		const std::string_view& mimeType,
		const std::string_view& content) {
		if (state_ != State::Initial) {
			throw LogicException(CPV_CODEINFO, "can't set body twice");
		}
		str_.append("Content-Type: ").append(mimeType).append("\r\n");
		str_.append("Content-Length: ");
		dumpIntToDec(content.size(), str_);
		str_.append("\r\n\r\n");
		state_ = State::HeaderFinished;
		str_.append(content);
		return *this;
	}

	/** Send this http request */
	seastar::future<HttpClientResponse> HttpClientRequest::send(HttpClient& client) {
		return client.send(str_);
	}

	/** Constructor */
	HttpClientRequest::HttpClientRequest(
		const std::string_view& method,
		const std::string_view& path,
		const std::string_view& host) :
		state_(State::Initial),
		str_() {
		str_.append(method).append(" ").append(path).append(" ").append("HTTP/1.1\r\n");
		addHeader("Host", host);
	}
}

