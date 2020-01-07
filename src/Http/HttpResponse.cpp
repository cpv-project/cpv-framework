#include <unordered_map>
#include <vector>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Http/HttpResponse.hpp>

namespace cpv {
	/** Members of HttpResponse */
	class HttpResponseData {
	public:
		SharedString version;
		SharedString statusCode;
		SharedString statusMessage;
		HttpResponseHeaders headers;
		Reusable<OutputStreamBase> bodyStream;
		
		HttpResponseData() :
			version(),
			statusCode(),
			statusMessage(),
			headers(),
			bodyStream() { }
		
		void freeResources() {
			version.clear();
			statusCode.clear();
			statusMessage.clear();
			headers.clear();
			bodyStream = Reusable<OutputStreamBase>();
		}
		
		static void reset() { }
	};
	
	/** Increase free list size */
	template <>
	const constexpr std::size_t ReusableStorageCapacity<HttpResponseData> = 28232;
	
	/** The storage of HttpResponseData */
	template <>
	thread_local ReusableStorageType<HttpResponseData>
		ReusableStorageInstance<HttpResponseData>;
	
	/** Get the http version string */
	const SharedString& HttpResponse::getVersion() const& {
		return data_->version;
	}
	
	/** Set the http version string */
	void HttpResponse::setVersion(SharedString&& version) {
		data_->version = std::move(version);
	}
	
	/** Get the status code */
	const SharedString& HttpResponse::getStatusCode() const& {
		return data_->statusCode;
	}
	
	/** Set the status code */
	void HttpResponse::setStatusCode(SharedString&& statusCode) {
		data_->statusCode = std::move(statusCode);
	}
	
	/** Get the reason message of status code */
	const SharedString& HttpResponse::getStatusMessage() const& {
		return data_->statusMessage;
	}
	
	/** Set the reason message of status code */
	void HttpResponse::setStatusMessage(SharedString&& statusMessage) {
		data_->statusMessage = std::move(statusMessage);
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
	void HttpResponse::setHeader(SharedString&& key, SharedString&& value) {
		data_->headers.setHeader(std::move(key), std::move(value));
	}
	
	/** Get response body output stream */
	const Reusable<OutputStreamBase>& HttpResponse::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set response body output stream */
	void HttpResponse::setBodyStream(Reusable<OutputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
	}
	
	/** Constructor */
	HttpResponse::HttpResponse() :
		data_(makeReusable<HttpResponseData>()) { }
	
	/** Constructor for null response */
	HttpResponse::HttpResponse(nullptr_t) :
		data_(Reusable<HttpResponseData>()) { }
}

