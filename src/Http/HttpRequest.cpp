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
	void HttpRequest::setMethod(const std::string_view& method) {
		data_->method = method;
	}
	
	/** Get the request url */
	std::string_view HttpRequest::getUrl() const& {
		return data_->url;
	}
	
	/** Set the request url */
	void HttpRequest::setUrl(const std::string_view& url) {
		data_->url = url;
	}
	
	/** Get the http version string */
	std::string_view HttpRequest::getVersion() const& {
		return data_->version;
	}
	
	/** Set the http version string */
	void HttpRequest::setVersion(const std::string_view& version) {
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
	void HttpRequest::setHeader(const std::string_view& key, const std::string_view& value) {
		data_->headers.setHeader(key, value);
	}
	
	/** Get underlying buffers */
	HttpRequest::UnderlyingBuffersType& HttpRequest::getUnderlyingBuffers() & {
		return data_->underlyingBuffers;
	}
	
	/** Get underlying buffers */
	const HttpRequest::UnderlyingBuffersType& HttpRequest::getUnderlyingBuffers() const& {
		return data_->underlyingBuffers;
	}
	
	/** Add underlying buffer that owns the storage of string views */
	void HttpRequest::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		data_->underlyingBuffers.emplace_back(std::move(buf));
	}
	
	/** Get request body input stream */
	const Reusable<InputStreamBase>& HttpRequest::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set request body input stream */
	void HttpRequest::setBodyStream(Reusable<InputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
	}
	
	/** Constructor */
	HttpRequest::HttpRequest() :
		data_(makeReusable<HttpRequestData>()) { }
}

