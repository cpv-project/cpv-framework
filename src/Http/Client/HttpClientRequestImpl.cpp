#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include "HttpClientRequestImpl.hpp"

namespace cpv {
	/** For Object */
	void HttpClientRequestImpl::reset(
		const std::string_view& method,
		const std::string_view& path,
		const std::string_view& host) {
		state_ = State::Initial;
		str_.resize(0);
		str_.append(method).append(" ").append(path).append(" ").append("HTTP/1.1\r\n");
		addHeader("Host", host);
	}

	/** For Object */
	void HttpClientRequestImpl::freeResources() { }

	/** Add a header */
	void HttpClientRequestImpl::addHeader(
		const std::string_view& key,
		const std::string_view& value) {
		if (state_ != State::Initial) {
			throw LogicException(CPV_CODEINFO, "can't add header after body is set");
		}
		str_.append(key).append(": ").append(value).append("\r\n");
	}

	/** Set the body of this request */
	void HttpClientRequestImpl::setBody(
		const std::string_view& mimeType,
		const std::string_view& content) {
		if (state_ != State::Initial) {
			throw LogicException(CPV_CODEINFO, "can't set body twice");
		}
		// TODO: support Accept-Encoding and gzip decompression
		if (str_.find("User-Agent") == std::string::npos) {
			str_.append("User-Agent: cpv-http-client " CPV_FRAMEWORK_VERSION_NUMBER "\r\n");
		}
		if (!mimeType.empty() || !content.empty()) {
			str_.append("Content-Type: ").append(mimeType).append("\r\n");
			str_.append("Content-Length: ");
			dumpIntToDec(content.size(), str_);
			str_.append("\r\n\r\n");
			str_.append(content);
		} else {
			str_.append("Content-Length: 0\r\n\r\n");
		}
		state_ = State::HeaderFinished;
	}

	/** Get the full content of this request */
	const std::string_view HttpClientRequestImpl::str() const& {
		if (state_ == State::Initial) {
			throw LogicException(CPV_CODEINFO, "please call setBody(\"\", \"\") if the body is empty");
		}
		return str_;
	}

	/** Constructor */
	HttpClientRequestImpl::HttpClientRequestImpl() :
		state_(State::Initial),
		str_() { }
}

