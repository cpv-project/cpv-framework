#include <cstdlib>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Http/Client/HttpClientResponse.hpp>
#include <CPVFramework/Http/Client/HttpClient.hpp>

namespace cpv {
	namespace {
		/** Example: HTTP/1.1 200 OK */
		void parseFirstLine(const char*& ptr, const char* end, std::size_t& status) {
			const char* str = ptr;
			for (;str < end && *str != ' '; ++str) { }
			if (*str == ' ') {
				++str;
				const char* strEnd = str;
				for (;strEnd < end && *strEnd != ' '; ++strEnd) { }
				if (*strEnd == ' ') {
					status = std::strtoul(str, nullptr, 10);
					for (ptr = strEnd + 1; ptr < end && *ptr != '\n'; ++ptr) { }
					if (*ptr == '\n') {
						++ptr;
						return; // success
					}
				}
			}
			status = static_cast<std::size_t>(-1); // failed
		}

		/** Example: Content-Type: text/html */
		void parseHeaders(const char*& ptr, const char* end,
			std::unordered_map<std::string_view, std::string_view>& headers) {
			while (ptr < end) {
				const char* keyBegin = ptr;
				const char* keyEnd = keyBegin;
				for (; keyEnd < end && *keyEnd != ':'; ++keyEnd) { }
				if (keyEnd >= end) {
					break; // failed
				}
				const char* valueBegin = keyEnd + 1;
				for (; valueBegin < end && *valueBegin == ' '; ++valueBegin) { }
				const char* valueEnd = valueBegin;
				for (; valueEnd < end && *valueEnd != '\n'; ++valueEnd) { }
				if (valueEnd >= end) {
					break; // failed
				}
				ptr = valueEnd + 1; // next line
				for (; valueEnd > valueBegin && valueEnd[-1] == '\r'; --valueEnd) { }
				for (; valueEnd > valueBegin && valueEnd[-1] == ' '; --valueEnd) { }
				headers.emplace(
					std::string_view(keyBegin, keyEnd - keyBegin),
					std::string_view(valueBegin, valueEnd - valueBegin));
			}
		}

		/** Example: Content-Length: 123 */
		void setContentLength(const std::string_view& contentLengthStr, std::size_t& contentLength) {
			if (contentLengthStr.empty()) {
				// header didn't tell content length, use maximum value
				contentLength = static_cast<std::size_t>(-1);
			} else {
				const char* str = contentLengthStr.data();
				contentLength = std::strtoul(str, nullptr, 10);
			}
		}
	}

	/** Get a single header value by key, return empty if not exist */
	const std::string_view HttpClientResponse::getHeader(const std::string_view& key) const& {
		auto it = headers_.find(key);
		if (it == headers_.end()) {
			return std::string_view();
		}
		return it->second;
	}

	/** Append received data, return whether is all content received */
	bool HttpClientResponse::appendReceived(const std::string_view& buf) {
		if (buf.size() == 0) {
			// connection closed, maybe chunked response
			return true;
		} else if (state_ == State::Initial) {
			headerStr_.append(buf.data(), buf.size());
 			std::size_t pos = headerStr_.find("\r\n\r\n");
			if (pos != std::string::npos) {
				bodyStr_.append(headerStr_.begin() + pos + 4, headerStr_.end());
				headerStr_.resize(pos + 2);
				const char* ptr = headerStr_.data();
				const char* end = ptr + headerStr_.size();
				parseFirstLine(ptr, end, status_);
				parseHeaders(ptr, end, headers_);
				setContentLength(getHeader("Content-Length"), contentLength_);
				state_ = State::HeaderFinished;
				return bodyStr_.size() >= contentLength_;
			} else {
				return false;
			}
		} else if (state_ == State::HeaderFinished) {
			bodyStr_.append(buf.data(), buf.size());
			return bodyStr_.size() >= contentLength_;
		}
		throw LogicException(CPV_CODEINFO, "invalid state:", static_cast<std::size_t>(state_));
	}

	/** Constructor */
	HttpClientResponse::HttpClientResponse() :
		state_(State::Initial),
		status_(0),
		contentLength_(0),
		headers_(),
		headerStr_(),
		bodyStr_() { }
}

