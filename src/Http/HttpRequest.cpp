#include <string_view>
#include <unordered_map>
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Http/HttpRequest.hpp>

namespace cpv {
	/** Members of HttpRequest */
	class HttpRequestData {
	public:
		std::string_view method;
		std::string_view url;
		std::string_view version;
		HttpRequestHeaders headers;
		Reusable<InputStreamBase> bodyStream;
		// mutable for lazy parse
		mutable std::string_view sourceOfUri;
		mutable HttpRequestUri uri;
		mutable std::string_view sourceOfCookies;
		mutable HttpRequestCookies cookies;
		HttpRequest::UnderlyingBuffersType underlyingBuffers;
		
		HttpRequestData() :
			method(),
			version(),
			headers(),
			bodyStream(),
			uri(),
			underlyingBuffers() { }
		
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
			underlyingBuffers.clear();
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
			auto cookieHeader = headers.getCookie();
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
	std::string_view HttpRequest::addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
		std::string_view view(buf.get(), buf.size());
		data_->underlyingBuffers.emplace_back(std::move(buf));
		return view;
	}
	
	/** Constructor */
	HttpRequest::HttpRequest() :
		data_(makeReusable<HttpRequestData>()) { }
	
	/** Constructor for null request */
	HttpRequest::HttpRequest(nullptr_t) :
		data_(Reusable<HttpRequestData>()) { }
}

