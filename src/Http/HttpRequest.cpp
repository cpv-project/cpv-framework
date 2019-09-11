#include <CPVFramework/Http/HttpRequest.hpp>
#include "./HttpRequestData.hpp"

namespace cpv {
	/** The storage of HttpRequestData */
	template <>
	thread_local ReusableStorageType<HttpRequestData>
		ReusableStorageInstance<HttpRequestData>;
	
	/** Get the request method */
	std::string_view HttpRequest::getMethod() const& {
		return data_->method;
	}
	
	/* Set the request method */
	void HttpRequest::setMethod(std::string_view method) {
		data_->method = method;
	}
	
	/** Get the request url */
	std::string_view HttpRequest::getUrl() const& {
		return data_->url;
	}
	
	/** Set the request url */
	void HttpRequest::setUrl(std::string_view url) {
		data_->url = url;
	}
	
	/** Get the http version string */
	std::string_view HttpRequest::getVersion() const& {
		return data_->version;
	}
	
	/** Set the http version string */
	void HttpRequest::setVersion(std::string_view version) {
		data_->version = version;
	}
	
	/** Get request headers */
	HttpRequestHeaders& HttpRequest::getHeaders() & {
		return data_->headers;
	}
	
	/** Get request headers */
	const HttpRequestHeaders& HttpRequest::getHeaders() const& {
		return data_->headers;
	}
	
	/** Set request header */
	void HttpRequest::setHeader(std::string_view key, std::string_view value) {
		data_->headers.setHeader(key, value);
	}
	
	/** Get underlying buffers */
	HttpRequest::UnderlyingBuffersType& HttpRequest::getUnderlyingBuffers() & {
		return data_->underlyingBuffers;
	}
	
	/** Get the uri instance parsed from url */
	HttpRequestUri& HttpRequest::getUri() & {
		data_->ensureUriUpdated();
		return data_->uri;
	}
	
	/** Get the uri instance parsed from url */
	const HttpRequestUri& HttpRequest::getUri() const& {
		data_->ensureUriUpdated();
		return data_->uri;
	}
	
	/** Get the cookies collection parsed from Cookie header */
	HttpRequestCookies& HttpRequest::getCookies() & {
		data_->ensureCookiesUpdated();
		return data_->cookies;
	}
	
	/** Get the cookies collection parsed from Cookie header */
	const HttpRequestCookies& HttpRequest::getCookies() const& {
		data_->ensureCookiesUpdated();
		return data_->cookies;
	}
	
	/** Get request body input stream */
	const Reusable<InputStreamBase>& HttpRequest::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set request body input stream */
	void HttpRequest::setBodyStream(Reusable<InputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
	}
	
	/** Get underlying buffers */
	const HttpRequest::UnderlyingBuffersType& HttpRequest::getUnderlyingBuffers() const& {
		return data_->underlyingBuffers;
	}
	
	/** Add underlying buffer that owns the storage of string views */
	void HttpRequest::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		data_->underlyingBuffers.emplace_back(std::move(buf));
	}
	
	/** Constructor */
	HttpRequest::HttpRequest() :
		data_(makeReusable<HttpRequestData>()) { }
	
	/** Constructor for null request */
	HttpRequest::HttpRequest(nullptr_t) :
		data_(Reusable<HttpRequestData>()) { }
}

