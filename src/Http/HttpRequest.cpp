#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Http/HttpRequest.hpp>

namespace cpv {
	/** Members of HttpRequest */
	class HttpRequestData {
	public:
		SharedString method;
		SharedString url;
		SharedString version;
		HttpRequestHeaders headers;
		Reusable<InputStreamBase> bodyStream;
		// mutable for lazy parse
		mutable std::string_view sourceOfUri;
		mutable HttpRequestUri uri;
		mutable std::string_view sourceOfCookies;
		mutable HttpRequestCookies cookies;
		
		HttpRequestData() :
			method(),
			url(),
			version(),
			headers(),
			bodyStream(),
			sourceOfUri(),
			uri(),
			sourceOfCookies(),
			cookies() { }
		
		void freeResources() {
			method = {};
			url = {};
			version = {};
			headers.clear();
			bodyStream = Reusable<InputStreamBase>();
			sourceOfUri = {};
			uri.clear();
			sourceOfCookies = {};
			cookies.clear();
		}
		
		inline void ensureUriUpdated() const {
			if (sourceOfUri.empty()) {
				sourceOfUri = url;
				uri.parse(url);
			} else if (sourceOfUri.data() != url.data()) {
				// compare pointer directly
				sourceOfUri = url;
				uri.clear();
				uri.parse(url);
			}
		}
		
		inline void ensureCookiesUpdated() const {
			auto& cookieHeader = headers.getCookie();
			if (sourceOfCookies.empty()) {
				sourceOfCookies = cookieHeader;
				cookies.parse(cookieHeader);
			} else if (sourceOfCookies.data() != cookieHeader.data()) {
				// compare pointer directly
				sourceOfCookies = cookieHeader;
				cookies.clear();
				cookies.parse(cookieHeader);
			}
		}
		
		static void reset() { }
	};
	
	/** Increase free list size */
	template <>
	const constexpr std::size_t ReusableStorageCapacity<HttpRequestData> = 28232;
	
	/** The storage of HttpRequestData */
	template <>
	thread_local ReusableStorageType<HttpRequestData>
		ReusableStorageInstance<HttpRequestData>;
	
	/** Get the request method */
	const SharedString& HttpRequest::getMethod() const& {
		return data_->method;
	}
	
	/* Set the request method */
	void HttpRequest::setMethod(SharedString&& method) {
		data_->method = std::move(method);
	}
	
	/** Get the request url */
	const SharedString& HttpRequest::getUrl() const& {
		return data_->url;
	}
	
	/** Set the request url */
	void HttpRequest::setUrl(SharedString&& url) {
		data_->url = std::move(url);
	}
	
	/** Get the http version string */
	const SharedString& HttpRequest::getVersion() const& {
		return data_->version;
	}
	
	/** Set the http version string */
	void HttpRequest::setVersion(SharedString&& version) {
		data_->version = std::move(version);
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
	void HttpRequest::setHeader(SharedString&& key, SharedString&& value) {
		data_->headers.setHeader(std::move(key), std::move(value));
	}
	
	/** Get request body input stream */
	const Reusable<InputStreamBase>& HttpRequest::getBodyStream() const& {
		return data_->bodyStream;
	}
	
	/** Set request body input stream */
	void HttpRequest::setBodyStream(Reusable<InputStreamBase>&& bodyStream) {
		data_->bodyStream = std::move(bodyStream);
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
	
	/** Constructor */
	HttpRequest::HttpRequest() :
		data_(makeReusable<HttpRequestData>()) { }
	
	/** Constructor for null request */
	HttpRequest::HttpRequest(nullptr_t) :
		data_(Reusable<HttpRequestData>()) { }
}

