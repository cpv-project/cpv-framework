#include <CPVFramework/Http/HttpResponse.hpp>
#include "./HttpResponseData.hpp"

namespace cpv {
	/** Get the http version string */
	std::string_view HttpResponse::getVersion() const& {
		return data_->version;
	}
	
	/** Set the http version string */
	void HttpResponse::setVersion(std::string_view version) {
		data_->version = version;
	}
	
	/** Get the status code */
	std::string_view HttpResponse::getStatusCode() const& {
		return data_->statusCode;
	}
	
	/** Set the status code */
	void HttpResponse::setStatusCode(std::string_view statusCode) {
		data_->statusCode = statusCode;
	}
	
	/** Get the reason message of status code */
	std::string_view HttpResponse::getStatusMessage() const& {
		return data_->statusMessage;
	}
	
	/** Set the reason message of status code */
	void HttpResponse::setStatusMessage(std::string_view statusMessage) {
		data_->statusMessage = statusMessage;
	}
	
	/** Get response headers */
	std::unordered_map<std::string_view, std::string_view>& HttpResponse::getHeaders() & {
		return data_->headers;
	}
	
	/** Get response headers */
	const std::unordered_map<std::string_view, std::string_view>& HttpResponse::getHeaders() const& {
		return data_->headers;
	}
	
	/** Set response header */
	void HttpResponse::setHeader(std::string_view key, std::string_view value) {
		data_->headers.insert_or_assign(key, value);
	}
	
	/** Add underlying buffer that owns the storage of string views */
	std::string_view HttpResponse::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		auto& bufRef = data_->underlyingBuffers.emplace_back(std::move(buf));
		return std::string_view(bufRef.get(), bufRef.size());
	}
	
	/** Get response body output stream */
	const Object<OutputStreamBase>& HttpResponse::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set response body output stream */
	void HttpResponse::setBodyStream(Object<OutputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
	}
	
	/** Constructor */
	HttpResponse::HttpResponse() :
		data_(makeObject<HttpResponseData>()) { }
}

