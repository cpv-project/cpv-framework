#include <unordered_map>
#include <CPVFramework/Http/HttpRequestHeaders.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	class HttpRequestHeaders::Internal {
	public:
		using FixedMembersType = std::unordered_map<std::string_view, SharedString(HttpRequestHeaders::*)>;
		static FixedMembersType FixedMembers;
	};

	HttpRequestHeaders::Internal::FixedMembersType HttpRequestHeaders::Internal::FixedMembers({
		{ constants::Host, &HttpRequestHeaders::host_ },
		{ constants::ContentType, &HttpRequestHeaders::contentType_ },
		{ constants::ContentLength, &HttpRequestHeaders::contentLength_ },
		{ constants::Connection, &HttpRequestHeaders::connection_ },
		{ constants::Pragma, &HttpRequestHeaders::pragma_ },
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
	void HttpRequestHeaders::setHeader(SharedString&& key, SharedString&& value) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = std::move(value);
		} else {
			remainHeaders_.insert_or_assign(std::move(key), std::move(value));
		}
	}
	
	/** Get header value */
	SharedString HttpRequestHeaders::getHeader(const SharedString& key) const {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			return (this->*(it->second)).share();
		} else {
			auto rit = remainHeaders_.find(key);
			if (rit != remainHeaders_.end()) {
				return rit->second.share();
			}
			return {};
		}
	}
	
	/** Remove header */
	void HttpRequestHeaders::removeHeader(const SharedString& key) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = {};
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
		host_.clear();
		contentType_.clear();
		contentLength_.clear();
		connection_.clear();
		pragma_.clear();
		upgradeInsecureRequests_.clear();
		dnt_.clear();
		userAgent_.clear();
		accept_.clear();
		acceptEncoding_.clear();
		acceptLanguage_.clear();
		cookie_.clear();
		xRequestedWith_.clear();
	}
	
	/** Constructor */
	HttpRequestHeaders::HttpRequestHeaders() :
		remainHeaders_(),
		host_(),
		contentType_(),
		contentLength_(),
		connection_(),
		pragma_(),
		upgradeInsecureRequests_(),
		dnt_(),
		userAgent_(),
		accept_(),
		acceptEncoding_(),
		acceptLanguage_(),
		cookie_(),
		xRequestedWith_() { }
}

