#pragma once
#include "../Allocators/StackAllocator.hpp"
#include "../Utility/SharedString.hpp"
#include "./HttpConstantStrings.hpp"

namespace cpv {
	/** Headers collection for http request */
	class HttpRequestHeaders {
	public:
		// getters and setters for fixed members
		const SharedString& getHost() const& { return host_; }
		const SharedString& getContentType() const& { return contentType_; }
		const SharedString& getContentLength() const& { return contentLength_; }
		const SharedString& getConnection() const& { return connection_; }
		const SharedString& getPragma() const& { return pragma_; }
		const SharedString& getUpgradeInsecureRequests() const& { return upgradeInsecureRequests_; }
		const SharedString& getDNT() const& { return dnt_; }
		const SharedString& getUserAgent() const& { return userAgent_; }
		const SharedString& getAccept() const& { return accept_; }
		const SharedString& getAcceptEncoding() const& { return acceptEncoding_; }
		const SharedString& getAcceptLanguage() const& { return acceptLanguage_; }
		const SharedString& getCookie() const& { return cookie_; }
		const SharedString& getXRequestedWith() const& { return xRequestedWith_; }
		void setHost(SharedString&& value) { host_ = std::move(value); }
		void setContentType(SharedString&& value) { contentType_ = std::move(value); }
		void setContentLength(SharedString&& value) { contentLength_ = std::move(value); }
		void setConnection(SharedString&& value) { connection_ = std::move(value); }
		void setPragma(SharedString&& value) { pragma_ = std::move(value); }
		void setUpgradeInsecureRequests(SharedString&& value) { upgradeInsecureRequests_ = std::move(value); }
		void setDNT(SharedString&& value) { dnt_ = std::move(value); }
		void setUserAgent(SharedString&& value) { userAgent_ = std::move(value); }
		void setAccept(SharedString&& value) { accept_ = std::move(value); }
		void setAcceptEncoding(SharedString&& value) { acceptEncoding_ = std::move(value); }
		void setAcceptLanguage(SharedString&& value) { acceptLanguage_ = std::move(value); }
		void setCookie(SharedString&& value) { cookie_ = std::move(value); }
		void setXRequestedWith(SharedString&& value) { xRequestedWith_ = std::move(value); }
		
		/** Apply function to all headers */
		template <class Func>
		void foreach(const Func& func) const {
			if (!host_.empty()) { func(constants::Host, host_); }
			if (!contentType_.empty()) { func(constants::ContentType, contentType_); }
			if (!contentLength_.empty()) { func(constants::ContentLength, contentLength_); }
			if (!connection_.empty()) { func(constants::Connection, connection_); }
			if (!pragma_.empty()) { func(constants::Pragma, pragma_); }
			if (!upgradeInsecureRequests_.empty()) { func(constants::UpgradeInsecureRequests, upgradeInsecureRequests_); }
			if (!dnt_.empty()) { func(constants::DNT, dnt_); }
			if (!userAgent_.empty()) { func(constants::UserAgent, userAgent_); }
			if (!accept_.empty()) { func(constants::Accept, accept_); }
			if (!acceptEncoding_.empty()) { func(constants::AcceptEncoding, acceptEncoding_); }
			if (!acceptLanguage_.empty()) { func(constants::AcceptLanguage, acceptLanguage_); }
			if (!cookie_.empty()) { func(constants::Cookie, cookie_); }
			if (!xRequestedWith_.empty()) { func(constants::XRequestedWith, xRequestedWith_); }
			for (auto& pair : remainHeaders_) {
				func(pair.first, pair.second);
			}
		}
		
		/** Set header value */
		void setHeader(SharedString&& key, SharedString&& value);
		
		/** Get header value, return empty string if key not exists */
		SharedString getHeader(const SharedString& key) const;
		
		/** Remove header */
		void removeHeader(const SharedString& key);
		
		/** Get maximum count of headers, may greater than actual count */
		std::size_t maxSize() const;
		
		/** Clear headers in this collection  */
		void clear();
		
	private:
		/** Constructor */
		HttpRequestHeaders();
		
		// make auto generated constructors and assign operators private
		HttpRequestHeaders(const HttpRequestHeaders&) = default;
		HttpRequestHeaders(HttpRequestHeaders&&) = default;
		HttpRequestHeaders& operator=(const HttpRequestHeaders&) = default;
		HttpRequestHeaders& operator=(HttpRequestHeaders&&) = default;
		
		class Internal;
		friend class HttpRequestData;
		
	private:
		StackAllocatedMap<SharedString, SharedString, 3> remainHeaders_;
		SharedString host_;
		SharedString contentType_;
		SharedString contentLength_;
		SharedString connection_;
		SharedString pragma_;
		SharedString upgradeInsecureRequests_;
		SharedString dnt_;
		SharedString userAgent_;
		SharedString accept_;
		SharedString acceptEncoding_;
		SharedString acceptLanguage_;
		SharedString cookie_;
		SharedString xRequestedWith_;
	};
}

