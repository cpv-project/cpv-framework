#pragma once
#include <utility>
#include "../Allocators/StackAllocator.hpp"
#include "../Utility/SharedString.hpp"
#include "../Utility/Packet.hpp"
#include "./HttpConstantStrings.hpp"

namespace cpv {
	/** Headers collection for http response */
	class HttpResponseHeaders {
	public:
		using AdditionHeadersType = StackAllocatedVector<
			std::pair<SharedString, SharedString>, 3>;
		
		// getters and setters for fixed members
		const SharedString& getDate() const& { return date_; }
		const SharedString& getContentType() const& { return contentType_; }
		const SharedString& getContentLength() const& { return contentLength_; }
		const SharedString& getContentEncoding() const& { return contentEncoding_; }
		const SharedString& getTransferEncoding() const& { return transferEncoding_; }
		const SharedString& getConnection() const& { return connection_; }
		const SharedString& getServer() const& { return server_; }
		const SharedString& getVary() const& { return vary_; }
		const SharedString& getETag() const& { return etag_; }
		const SharedString& getCacheControl() const& { return cacheControl_; }
		const SharedString& getExpires() const& { return expires_; }
		const SharedString& getLastModified() const& { return lastModified_; }
		void setDate(SharedString&& value) { date_ = std::move(value); }
		void setContentType(SharedString&& value) { contentType_ = std::move(value); }
		void setContentLength(SharedString&& value) { contentLength_ = std::move(value); }
		void setContentEncoding(SharedString&& value) { contentEncoding_ = std::move(value); }
		void setTransferEncoding(SharedString&& value) { transferEncoding_ = std::move(value); }
		void setConnection(SharedString&& value) { connection_ = std::move(value); }
		void setServer(SharedString&& value) { server_ = std::move(value); }
		void setVary(SharedString&& value) { vary_ = std::move(value); }
		void setETag(SharedString&& value) { etag_ = std::move(value); }
		void setCacheControl(SharedString&& value) { cacheControl_ = std::move(value); }
		void setExpires(SharedString&& value) { expires_ = std::move(value); }
		void setLastModified(SharedString&& value) { lastModified_ = std::move(value); }
		
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
			if (!expires_.empty()) { func(constants::Expires, expires_); }
			if (!lastModified_.empty()) { func(constants::LastModified, lastModified_); }
			for (auto& pair : remainHeaders_) {
				func(pair.first, pair.second);
			}
			for (auto& pair : additionHeaders_) {
				func(pair.first, pair.second);
			}
		}
		
		/**
		 * Append all headers to packet fragments for http 1.
		 * notice it will append crlf to begin but not to end.
		 */
		void appendToHttp1Packet(Packet::MultipleFragments& fragments);
		
		/** Set header value */
		void setHeader(SharedString&& key, SharedString&& value);
		
		/** Get header value, return empty string if key not exists */
		SharedString getHeader(const SharedString& key) const;
		
		/** Remove header */
		void removeHeader(const SharedString& key);
		
		/** Add header that may occurs multiple times */
		void addAdditionHeader(SharedString&& key, SharedString&& value);
		
		/** Get headers that may occurs multiple times */
		AdditionHeadersType& getAdditionHeaders() &;
		const AdditionHeadersType& getAdditionHeaders() const&;
		
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
		StackAllocatedMap<SharedString, SharedString, 3> remainHeaders_;
		AdditionHeadersType additionHeaders_; // mostly for Set-Cookie
		SharedString date_;
		SharedString contentType_;
		SharedString contentLength_;
		SharedString contentEncoding_;
		SharedString transferEncoding_;
		SharedString connection_;
		SharedString server_;
		SharedString vary_;
		SharedString etag_;
		SharedString cacheControl_;
		SharedString expires_;
		SharedString lastModified_;
	};
}

