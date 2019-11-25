#pragma once
#include "../Allocators/StackAllocator.hpp"
#include "../Stream/OutputStreamBase.hpp"
#include "../Utility/Reusable.hpp"
#include "../Utility/SharedString.hpp"
#include "./HttpResponseHeaders.hpp"

namespace cpv {
	/** Members of HttpResponse */
	class HttpResponseData;
	
	/**
	 * Contains headers, body and addition informations of a http response.
	 * It should contains only data so it can be mock easily.
	 */
	class HttpResponse {
	public:
		/** Get the http version string, e.g. "HTTP/1.1" */
		const SharedString& getVersion() const&;
		
		/** Set the http version string */
		void setVersion(SharedString&& version);
		
		/** Get the status code, e.g. "404" */
		const SharedString& getStatusCode() const&;
		
		/** Set the status code */
		void setStatusCode(SharedString&& statusCode);
		
		/** Get the reason message of status code, e.g. "Not Found" */
		const SharedString& getStatusMessage() const&;
		
		/** Set the reason message of status code */
		void setStatusMessage(SharedString&& statusMessage);
		
		/** Get response headers */
		HttpResponseHeaders& getHeaders() &;
		const HttpResponseHeaders& getHeaders() const&;
		
		/** Set response header */
		void setHeader(SharedString&& key, SharedString&& value);
		
		/** Get response body output stream, must check whether is null before access */
		const Reusable<OutputStreamBase>& getBodyStream() const&;
		
		/** Set response body output stream */
		void setBodyStream(Reusable<OutputStreamBase>&& bodyStream);
		
		/** Constructor */
		HttpResponse();
		
		/** Constructor for null response */
		explicit HttpResponse(nullptr_t);
		
	private:
		Reusable<HttpResponseData> data_;
	};
}

