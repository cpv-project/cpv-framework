#pragma once
#include <string>
#include <string_view>

namespace cpv {
	/** Interface of http request builder */
	class HttpClientRequest {
	public:
		/** Add a header */
		virtual void addHeader(
			const std::string_view& key,
			const std::string_view& value) = 0;

		/** Set the body of this request */
		virtual void setBody(
			const std::string_view& mimeType,
			const std::string_view& content) = 0;

		/** Get the full content of this request */
		virtual const std::string_view str() const& = 0;
	};
}

