#pragma once
#include <limits>
#include "../Stream/InputStreamBase.hpp"
#include "../Utility/Reusable.hpp"
#include "../Utility/SharedString.hpp"
#include "./HttpRequestHeaders.hpp"
#include "./HttpRequestUri.hpp"
#include "./HttpRequestCookies.hpp"

namespace cpv {
	/** Members of HttpRequest */
	class HttpRequestData;
	
	/**
	 * Contains headers, body and addition informations of a http request.
	 * It should contains only data so it can be mock easily.
	 */
	class HttpRequest {
	public:
		/** Get the request method, e.g. "GET" */
		const SharedString& getMethod() const&;
		
		/* Set the request method */
		void setMethod(SharedString&& method);
		
		/** Get the request url, e.g. "/test" */
		const SharedString& getUrl() const&;
		
		/** Set the request url */
		void setUrl(SharedString&& url);
		
		/** Get the http version string, e.g. "HTTP/1.1" */
		const SharedString& getVersion() const&;
		
		/** Set the http version string */
		void setVersion(SharedString&& version);
		
		/** Get request headers */
		HttpRequestHeaders& getHeaders() &;
		const HttpRequestHeaders& getHeaders() const&;
		
		/** Set request header */
		void setHeader(SharedString&& key, SharedString&& value);
		
		/** Get request body input stream, must check whether is null before access */
		const Reusable<InputStreamBase>& getBodyStream() const&;
		
		/** Set request body input stream */
		void setBodyStream(Reusable<InputStreamBase>&& bodyStream);
		
		/** Get the uri instance parsed from url */
		HttpRequestUri& getUri() &;
		const HttpRequestUri& getUri() const&;
		
		/** Get the cookies collection parsed from Cookie header */
		HttpRequestCookies& getCookies() &;
		const HttpRequestCookies& getCookies() const&;
		
		/** Constructor */
		HttpRequest();
		
		/** Constructor for null request */
		explicit HttpRequest(nullptr_t);

	private:
		Reusable<HttpRequestData> data_;
	};
}

