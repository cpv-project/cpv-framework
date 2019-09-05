#include <unordered_map>
#include <CPVFramework/Http/HttpRequestHeaders.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	class HttpRequestHeaders::Internal {
	public:
		using FixedMembersType = std::unordered_map<std::string_view, std::string_view(HttpRequestHeaders::*)>;
		static FixedMembersType FixedMembers;
	};

	HttpRequestHeaders::Internal::FixedMembersType HttpRequestHeaders::Internal::FixedMembers({
		{ constants::Host, &HttpRequestHeaders::host_ },
		{ constants::ContentType, &HttpRequestHeaders::contentType_ },
		{ constants::ContentLength, &HttpRequestHeaders::contentLength_ },
		{ constants::Connection, &HttpRequestHeaders::connection_ },
		{ constants::Pragma, &HttpRequestHeaders::pragma_ },
		{ constants::CacheControl, &HttpRequestHeaders::cacheControl_ },
		{ constants::UpgradeInsecureRequests, &HttpRequestHeaders::upgradeInsecureRequests_ },
		{ constants::DNT, &HttpRequestHeaders::dnt_ },
		{ constants::UserAgent, &HttpRequestHeaders::userAgent_ },
		{ constants::Accept, &HttpRequestHeaders::accept_ },
		{ constants::AcceptEncoding, &HttpRequestHeaders::acceptEncoding_ },
		{ constants::AcceptLanguage, &HttpRequestHeaders::acceptLanguage_ },
		{ constants::Cookie, &HttpRequestHeaders::cookie_ },
		{ constants::XRequestedWith, &HttpRequestHeaders::xRequestedWith_ }
	});

	/** Set header value */
	void HttpRequestHeaders::setHeader(const std::string_view& key, const std::string_view& value) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = value;
		} else {
			remainHeaders_.insert_or_assign(key, value);
		}
	}
	
	/** Get header value */
	std::string_view HttpRequestHeaders::getHeader(const std::string_view& key) const {
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
	void HttpRequestHeaders::removeHeader(const std::string_view& key) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = { };
		} else {
			remainHeaders_.erase(key);
		}
	}
	
	/** Get maximum count of headers, may greater than actual count */
	std::size_t HttpRequestHeaders::maxSize() const {
		return Internal::FixedMembers.size() + remainHeaders_.size();
	}
	
	/** Clear headers in this collection */
	void HttpRequestHeaders::clear() {
		remainHeaders_.clear();
		host_ = {};
		contentType_ = {};
		contentLength_ = {};
		connection_ = {};
		pragma_ = {};
		cacheControl_ = {};
		upgradeInsecureRequests_ = {};
		dnt_ = {};
		userAgent_ = {};
		accept_ = {};
		acceptEncoding_ = {};
		acceptLanguage_ = {};
		cookie_ = {};
		xRequestedWith_ = {};
	}
	
	/** Constructor */
	HttpRequestHeaders::HttpRequestHeaders() :
		remainHeaders_(),
		host_(),
		contentType_(),
		contentLength_(),
		connection_(),
		pragma_(),
		cacheControl_(),
		upgradeInsecureRequests_(),
		dnt_(),
		userAgent_(),
		accept_(),
		acceptEncoding_(),
		acceptLanguage_(),
		cookie_(),
		xRequestedWith_() { }
}

