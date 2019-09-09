#pragma once
#include <string_view>
#include "../Allocators/StackAllocator.hpp"
#include "./HttpConstantStrings.hpp"

namespace cpv {
	/**
	 * Headers collection for http request
	 * Notice: this collection only contains string_view, the storage is hold in HttpRequest
	 */
	class HttpRequestHeaders {
	public:
		// getters and setters for fixed members
		std::string_view getHost() const { return host_; }
		std::string_view getContentType() const { return contentType_; }
		std::string_view getContentLength() const { return contentLength_; }
		std::string_view getConnection() const { return connection_; }
		std::string_view getPragma() const { return pragma_; }
		std::string_view getCacheControl() const { return cacheControl_; }
		std::string_view getUpgradeInsecureRequests() const { return upgradeInsecureRequests_; }
		std::string_view getDNT() const { return dnt_; }
		std::string_view getUserAgent() const { return userAgent_; }
		std::string_view getAccept() const { return accept_; }
		std::string_view getAcceptEncoding() const { return acceptEncoding_; }
		std::string_view getAcceptLanguage() const { return acceptLanguage_; }
		std::string_view getCookie() const { return cookie_; }
		std::string_view getXRequestedWith() const { return xRequestedWith_; }
		void setHost(std::string_view value) { host_ = value; }
		void setContentType(std::string_view value) { contentType_ = value; }
		void setContentLength(std::string_view value) { contentLength_ = value; }
		void setConnection(std::string_view value) { connection_ = value; }
		void setPragma(std::string_view value) { pragma_ = value; }
		void setCacheControl(std::string_view value) { cacheControl_ = value; }
		void setUpgradeInsecureRequests(std::string_view value) { upgradeInsecureRequests_ = value; }
		void setDNT(std::string_view value) { dnt_ = value; }
		void setUserAgent(std::string_view value) { userAgent_ = value; }
		void setAccept(std::string_view value) { accept_ = value; }
		void setAcceptEncoding(std::string_view value) { acceptEncoding_ = value; }
		void setAcceptLanguage(std::string_view value) { acceptLanguage_ = value; }
		void setCookie(std::string_view value) { cookie_ = value; }
		void setXRequestedWith(std::string_view value) { xRequestedWith_ = value; }
		
		/** Apply function to all headers */
		template <class Func>
		void foreach(const Func& func) const {
			if (!host_.empty()) { func(constants::Host, host_); }
			if (!contentType_.empty()) { func(constants::ContentType, contentType_); }
			if (!contentLength_.empty()) { func(constants::ContentLength, contentLength_); }
			if (!connection_.empty()) { func(constants::Connection, connection_); }
			if (!pragma_.empty()) { func(constants::Pragma, pragma_); }
			if (!cacheControl_.empty()) { func(constants::CacheControl, cacheControl_); }
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
		void setHeader(std::string_view key, std::string_view value);
		
		/** Get header value */
		std::string_view getHeader(std::string_view key) const;
		
		/** Remove header */
		void removeHeader(std::string_view key);
		
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
		StackAllocatedMap<std::string_view, std::string_view, 3> remainHeaders_;
		std::string_view host_;
		std::string_view contentType_;
		std::string_view contentLength_;
		std::string_view connection_;
		std::string_view pragma_;
		std::string_view cacheControl_;
		std::string_view upgradeInsecureRequests_;
		std::string_view dnt_;
		std::string_view userAgent_;
		std::string_view accept_;
		std::string_view acceptEncoding_;
		std::string_view acceptLanguage_;
		std::string_view cookie_;
		std::string_view xRequestedWith_;
	};
}

