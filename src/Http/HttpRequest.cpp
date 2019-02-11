#include <CPVFramework/Http/HttpRequest.hpp>
#include "./HttpRequestData.hpp"

namespace cpv {
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
	std::unordered_map<std::string_view, std::string_view>& HttpRequest::getHeaders() & {
		return data_->headers;
	}
	
	/** Get request headers */
	const std::unordered_map<std::string_view, std::string_view>& HttpRequest::getHeaders() const& {
		return data_->headers;
	}
	
	/** Set request header */
	void HttpRequest::setHeader(const std::string_view& key, const std::string_view& value) {
		data_->headers.insert_or_assign(key, value);
	}
	
	/** Get underlying buffers */
	std::vector<seastar::temporary_buffer<char>>& HttpRequest::getUnderlyingBuffers() & {
		return data_->underlyingBuffers;
	}
	
	/** Get underlying buffers */
	const std::vector<seastar::temporary_buffer<char>>& HttpRequest::getUnderlyingBuffers() const& {
		return data_->underlyingBuffers;
	}
	
	/** Add underlying buffer that owns the storage of string views */
	void HttpRequest::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		data_->underlyingBuffers.emplace_back(std::move(buf));
	}
	
	/** Get request body input stream */
	const Object<InputStreamBase>& HttpRequest::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set request body input stream */
	void HttpRequest::setBodyStream(Object<InputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
	}
	
	/** Constructor */
	HttpRequest::HttpRequest() :
		data_(makeObject<HttpRequestData>()) { }
}

