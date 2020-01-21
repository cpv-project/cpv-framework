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
	
	/** Append all headers to packet fragments for http 1 */
	void HttpRequestHeaders::appendToHttp1Packet(Packet::MultipleFragments& fragments) {
		namespace cs = constants::with_crlf_colonspace;
		if (!host_.empty()) {
			fragments.append(cs::Host);
			fragments.append(host_.share());
		}
		if (!contentType_.empty()) {
			fragments.append(cs::ContentType);
			fragments.append(contentType_.share());
		}
		if (!contentLength_.empty()) {
			fragments.append(cs::ContentLength);
			fragments.append(contentLength_.share());
		}
		if (!connection_.empty()) {
			fragments.append(cs::Connection);
			fragments.append(connection_.share());
		}
		if (!pragma_.empty()) {
			fragments.append(cs::Pragma);
			fragments.append(pragma_.share());
		}
		if (!upgradeInsecureRequests_.empty()) {
			fragments.append(cs::UpgradeInsecureRequests);
			fragments.append(upgradeInsecureRequests_.share());
		}
		if (!dnt_.empty()) {
			fragments.append(cs::DNT);
			fragments.append(dnt_.share());
		}
		if (!userAgent_.empty()) {
			fragments.append(cs::UserAgent);
			fragments.append(userAgent_.share());
		}
		if (!accept_.empty()) {
			fragments.append(cs::Accept);
			fragments.append(accept_.share());
		}
		if (!acceptEncoding_.empty()) {
			fragments.append(cs::AcceptEncoding);
			fragments.append(acceptEncoding_.share());
		}
		if (!acceptLanguage_.empty()) {
			fragments.append(cs::AcceptLanguage);
			fragments.append(acceptLanguage_.share());
		}
		if (!cookie_.empty()) {
			fragments.append(cs::Cookie);
			fragments.append(cookie_.share());
		}
		if (!xRequestedWith_.empty()) {
			fragments.append(cs::XRequestedWith);
			fragments.append(xRequestedWith_.share());
		}
		for (auto& pair : remainHeaders_) {
			fragments.append(constants::CRLF);
			fragments.append(pair.first.share());
			fragments.append(constants::ColonSpace);
			fragments.append(pair.second.share());
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

