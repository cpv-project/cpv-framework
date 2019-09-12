#include <CPVFramework/Http/HttpResponse.hpp>
#include "./HttpResponseData.hpp"

namespace cpv {
	/** The storage of HttpResponseData */
	template <>
	thread_local ReusableStorageType<HttpResponseData>
		ReusableStorageInstance<HttpResponseData>;
	
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
	HttpResponseHeaders& HttpResponse::getHeaders() & {
		return data_->headers;
	}
	
	/** Get response headers */
	const HttpResponseHeaders& HttpResponse::getHeaders() const& {
		return data_->headers;
	}
	
	/** Set response header */
	void HttpResponse::setHeader(std::string_view key, std::string_view value) {
		data_->headers.setHeader(key, value);
	}
	
	/** Get response body output stream */
	const Reusable<OutputStreamBase>& HttpResponse::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set response body output stream */
	void HttpResponse::setBodyStream(Reusable<OutputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
	}
	
	/** Get underlying buffers */
	HttpResponse::UnderlyingBuffersType& HttpResponse::getUnderlyingBuffers() & {
		return data_->underlyingBuffers;
	}
	
	/** Get underlying buffers */
	const HttpResponse::UnderlyingBuffersType& HttpResponse::getUnderlyingBuffers() const& {
		return data_->underlyingBuffers;
	}
	
	/** Add underlying buffer that owns the storage of string views */
	void HttpResponse::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		data_->underlyingBuffers.emplace_back(std::move(buf));
	}
	
	/** Constructor */
	HttpResponse::HttpResponse() :
		data_(makeReusable<HttpResponseData>()) { }
	
	/** Constructor for null response */
	HttpResponse::HttpResponse(nullptr_t) :
		data_(Reusable<HttpResponseData>()) { }
}

