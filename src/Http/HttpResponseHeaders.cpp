#include <unordered_map>
#include <CPVFramework/Http/HttpResponseHeaders.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	class HttpResponseHeaders::Internal {
	public:
		using FixedMembersType = std::unordered_map<std::string_view, std::string_view(HttpResponseHeaders::*)>;
		static FixedMembersType FixedMembers;
	};
	
	HttpResponseHeaders::Internal::FixedMembersType HttpResponseHeaders::Internal::FixedMembers({
		{ constants::Date, &HttpResponseHeaders::date_ },
		{ constants::ContentType, &HttpResponseHeaders::contentType_ },
		{ constants::ContentLength, &HttpResponseHeaders::contentLength_ },
		{ constants::ContentEncoding, &HttpResponseHeaders::contentEncoding_ },
		{ constants::TransferEncoding, &HttpResponseHeaders::transferEncoding_ },
		{ constants::Connection, &HttpResponseHeaders::connection_ },
		{ constants::Server, &HttpResponseHeaders::server_ },
		{ constants::Vary, &HttpResponseHeaders::vary_ },
		{ constants::ETag, &HttpResponseHeaders::etag_ },
		{ constants::CacheControl, &HttpResponseHeaders::cacheControl_ },
		{ constants::SetCookie, &HttpResponseHeaders::setCookie_ },
		{ constants::Expires, &HttpResponseHeaders::expires_ },
		{ constants::LastModified, &HttpResponseHeaders::lastModified_ },
	});
	
	/** Set header value */
	void HttpResponseHeaders::setHeader(const std::string_view& key, const std::string_view& value) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = value;
		} else {
			remainHeaders_.insert_or_assign(key, value);
		}
	}
	
	/** Get header value */
	std::string_view HttpResponseHeaders::getHeader(const std::string_view& key) const {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			return this->*(it->second);
		} else {
			auto rit = remainHeaders_.find(key);
			if (rit != remainHeaders_.end()) {
				return rit->second;
			}
			return { };
		}
	}
	
	/** Remove header */
	void HttpResponseHeaders::removeHeader(const std::string_view& key) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = { };
		} else {
			remainHeaders_.erase(key);
		}
	}
	
	/** Get maximum count of headers, may greater than actual count */
	std::size_t HttpResponseHeaders::maxSize() const {
		return Internal::FixedMembers.size() + remainHeaders_.size();
	}
	
	/** Constructor */
	HttpResponseHeaders::HttpResponseHeaders() :
		remainHeaders_(),
		date_(),
		contentType_(),
		contentLength_(),
		contentEncoding_(),
		transferEncoding_(),
		connection_(),
		server_(),
		vary_(),
		etag_(),
		cacheControl_(),
		setCookie_(),
		expires_(),
		lastModified_() { }
}

