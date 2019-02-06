#include <CPVFramework/Http/HttpRequest.hpp>
#include "./HttpRequestData.hpp"

namespace cpv {
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
	std::unordered_map<std::string_view, std::string_view>& HttpRequest::getHeaders() & {
		return data_->headers;
	}
	
	/** Get request headers */
	const std::unordered_map<std::string_view, std::string_view>& HttpRequest::getHeaders() const& {
		return data_->headers;
	}
	
	/** Set request header */
	void HttpRequest::setHeader(std::string_view key, std::string_view value) {
		data_->headers.insert_or_assign(key, value);
	}
	
	/** Add underlying buffer that owns the storage of string views */
	std::string_view HttpRequest::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		auto& bufRef = data_->underlyingBuffers.emplace_back(std::move(buf));
		return std::string_view(bufRef.get(), bufRef.size());
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

