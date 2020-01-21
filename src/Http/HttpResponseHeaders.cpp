#include <unordered_map>
#include <CPVFramework/Http/HttpResponseHeaders.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	class HttpResponseHeaders::Internal {
	public:
		using FixedMembersType = std::unordered_map<std::string_view, SharedString(HttpResponseHeaders::*)>;
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
		{ constants::Expires, &HttpResponseHeaders::expires_ },
		{ constants::LastModified, &HttpResponseHeaders::lastModified_ },
	});
	
	/** Append all headers to packet fragments for http 1 */
	void HttpResponseHeaders::appendToHttp1Packet(Packet::MultipleFragments& fragments) {
		namespace cs = constants::with_crlf_colonspace;
		if (!date_.empty()) {
			fragments.append(cs::Date);
			fragments.append(date_.share());
		}
		if (!contentType_.empty()) {
			fragments.append(cs::ContentType);
			fragments.append(contentType_.share());
		}
		if (!contentLength_.empty()) {
			fragments.append(cs::ContentLength);
			fragments.append(contentLength_.share());
		}
		if (!contentEncoding_.empty()) {
			fragments.append(cs::ContentEncoding);
			fragments.append(contentEncoding_.share());
		}
		if (!transferEncoding_.empty()) {
			fragments.append(cs::TransferEncoding);
			fragments.append(transferEncoding_.share());
		}
		if (!connection_.empty()) {
			fragments.append(cs::Connection);
			fragments.append(connection_.share());
		}
		if (!server_.empty()) {
			fragments.append(cs::Server);
			fragments.append(server_.share());
		}
		if (!vary_.empty()) {
			fragments.append(cs::Vary);
			fragments.append(vary_.share());
		}
		if (!etag_.empty()) {
			fragments.append(cs::ETag);
			fragments.append(etag_.share());
		}
		if (!cacheControl_.empty()) {
			fragments.append(cs::CacheControl);
			fragments.append(cacheControl_.share());
		}
		if (!expires_.empty()) {
			fragments.append(cs::Expires);
			fragments.append(expires_.share());
		}
		if (!lastModified_.empty()) {
			fragments.append(cs::LastModified);
			fragments.append(lastModified_.share());
		}
		for (auto& pair : remainHeaders_) {
			fragments.append(constants::CRLF);
			fragments.append(pair.first.share());
			fragments.append(constants::ColonSpace);
			fragments.append(pair.second.share());
		}
		for (auto& pair : additionHeaders_) {
			fragments.append(constants::CRLF);
			fragments.append(pair.first.share());
			fragments.append(constants::ColonSpace);
			fragments.append(pair.second.share());
		}
	}
	
	/** Set header value */
	void HttpResponseHeaders::setHeader(SharedString&& key, SharedString&& value) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = std::move(value);
		} else {
			remainHeaders_.insert_or_assign(std::move(key), std::move(value));
		}
	}
	
	/** Get header value */
	SharedString HttpResponseHeaders::getHeader(const SharedString& key) const {
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
	void HttpResponseHeaders::removeHeader(const SharedString& key) {
		auto it = Internal::FixedMembers.find(key);
		if (CPV_LIKELY(it != Internal::FixedMembers.end())) {
			this->*(it->second) = {};
		} else {
			remainHeaders_.erase(key);
		}
	}
	
	/** Add header that may occurs multiple times */
	void HttpResponseHeaders::addAdditionHeader(
		SharedString&& key, SharedString&& value) {
		additionHeaders_.emplace_back(std::move(key), std::move(value));
	}
	
	/** Get headers that may occurs multiple times */
	HttpResponseHeaders::AdditionHeadersType&
	HttpResponseHeaders::getAdditionHeaders() & {
		return additionHeaders_;
	}
	
	/** Get headers that may occurs multiple times */
	const HttpResponseHeaders::AdditionHeadersType&
	HttpResponseHeaders::getAdditionHeaders() const& {
		return additionHeaders_;
	}
	
	/** Get maximum count of headers, may greater than actual count */
	std::size_t HttpResponseHeaders::maxSize() const {
		return (Internal::FixedMembers.size() +
			remainHeaders_.size() +
			additionHeaders_.size());
	}
	
	/** Clear headers in this collection */
	void HttpResponseHeaders::clear() {
		remainHeaders_.clear();
		additionHeaders_.clear();
		date_.clear();
		contentType_.clear();
		contentLength_.clear();
		contentEncoding_.clear();
		transferEncoding_.clear();
		connection_.clear();
		server_.clear();
		vary_.clear();
		etag_.clear();
		cacheControl_.clear();
		expires_.clear();
		lastModified_.clear();
	}
	
	/** Constructor */
	HttpResponseHeaders::HttpResponseHeaders() :
		remainHeaders_(),
		additionHeaders_(),
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
		expires_(),
		lastModified_() { }
}

