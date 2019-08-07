#pragma once
#include <string_view>
#include "../Allocators/StackAllocator.hpp"
#include "./HttpConstantStrings.hpp"

namespace cpv {
	/**
	 * Headers collection for http response
	 * Notice: this collection only contains string_view, the storage is hold in HttpResponse
	 */
	class HttpResponseHeaders {
	public:
		// getters and setters for fixed members
		std::string_view getDate() const { return date_; }
		std::string_view getContentType() const { return contentType_; }
		std::string_view getContentLength() const { return contentLength_; }
		std::string_view getContentEncoding() const { return contentEncoding_; }
		std::string_view getTransferEncoding() const { return transferEncoding_; }
		std::string_view getConnection() const { return connection_; }
		std::string_view getServer() const { return server_; }
		std::string_view getVary() const { return vary_; }
		std::string_view getETag() const { return etag_; }
		std::string_view getCacheControl() const { return cacheControl_; }
		std::string_view getSetCookie() const { return setCookie_; }
		std::string_view getExpires() const { return expires_; }
		std::string_view getLastModified() const { return lastModified_; }
		void setDate(const std::string_view& value) { date_ = value; }
		void setContentType(const std::string_view& value) { contentType_ = value; }
		void setContentLength(const std::string_view& value) { contentLength_ = value; }
		void setContentEncoding(const std::string_view& value) { contentEncoding_ = value; }
		void setTransferEncoding(const std::string_view& value) { transferEncoding_ = value; }
		void setConnection(const std::string_view& value) { connection_ = value; }
		void setServer(const std::string_view& value) { server_ = value; }
		void setVary(const std::string_view& value) { vary_ = value; }
		void setETag(const std::string_view& value) { etag_ = value; }
		void setCacheControl(const std::string_view& value) { cacheControl_ = value; }
		void setSetCookie(const std::string_view& value) { setCookie_ = value; }
		void setExpires(const std::string_view& value) { expires_ = value; }
		void setLastModified(const std::string_view& value) { lastModified_ = value; }
		
		/** Apply function to all headers */
		template <class Func>
		void foreach(const Func& func) const {
			if (!date_.empty()) { func(constants::Date, date_); }
			if (!contentType_.empty()) { func(constants::ContentType, contentType_); }
			if (!contentLength_.empty()) { func(constants::ContentLength, contentLength_); }
			if (!contentEncoding_.empty()) { func(constants::ContentEncoding, contentEncoding_); }
			if (!transferEncoding_.empty()) { func(constants::TransferEncoding, transferEncoding_); }
			if (!connection_.empty()) { func(constants::Connection, connection_); }
			if (!server_.empty()) { func(constants::Server, server_); }
			if (!vary_.empty()) { func(constants::Vary, vary_); }
			if (!etag_.empty()) { func(constants::ETag, etag_); }
			if (!cacheControl_.empty()) { func(constants::CacheControl, cacheControl_); }
			if (!setCookie_.empty()) { func(constants::SetCookie, setCookie_); }
			if (!expires_.empty()) { func(constants::Expires, expires_); }
			if (!lastModified_.empty()) { func(constants::LastModified, lastModified_); }
			for (auto& pair : remainHeaders_) {
				func(pair.first, pair.second);
			}
		}
		
		/** Set header value */
		void setHeader(const std::string_view& key, const std::string_view& value);
		
		/** Get header value */
		std::string_view getHeader(const std::string_view& key) const;
		
		/** Remove header */
		void removeHeader(const std::string_view& key);
		
		/** Get maximum count of headers, may greater than actual count */
		std::size_t maxSize() const;
		
		/** Clear headers in this collection  */
		void clear();
		
	private:
		/** Constructor */
		HttpResponseHeaders();
		
		// make auto generated constructors and assign operators private
		HttpResponseHeaders(const HttpResponseHeaders&) = default;
		HttpResponseHeaders(HttpResponseHeaders&&) = default;
		HttpResponseHeaders& operator=(const HttpResponseHeaders&) = default;
		HttpResponseHeaders& operator=(HttpResponseHeaders&&) = default;
		
		class Internal;
		friend class HttpResponseData;
		
	private:
		StackAllocatedMap<std::string_view, std::string_view, 3> remainHeaders_;
		std::string_view date_;
		std::string_view contentType_;
		std::string_view contentLength_;
		std::string_view contentEncoding_;
		std::string_view transferEncoding_;
		std::string_view connection_;
		std::string_view server_;
		std::string_view vary_;
		std::string_view etag_;
		std::string_view cacheControl_;
		std::string_view setCookie_;
		std::string_view expires_;
		std::string_view lastModified_;
	};
}

