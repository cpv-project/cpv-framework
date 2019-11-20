#pragma once
#include <string_view>
#include <limits>
#include <seastar/core/temporary_buffer.hh>
#include "../Allocators/StackAllocator.hpp"
#include "../Stream/InputStreamBase.hpp"
#include "../Utility/Reusable.hpp"
#include "../Utility/BufferUtils.hpp"
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
		using UnderlyingBuffersType = StackAllocatedVector<seastar::temporary_buffer<char>, 32>;
		
		/** Get the request method, e.g. "GET" */
		std::string_view getMethod() const&;
		
		/* Set the request method, must add underlying buffer first unless it's static string */
		void setMethod(std::string_view method);
		
		/** Get the request url, e.g. "/test" */
		std::string_view getUrl() const&;
		
		/** Set the request url, must add underlying buffer first unless it's static string */
		void setUrl(std::string_view url);
		
		/** Get the http version string, e.g. "HTTP/1.1" */
		std::string_view getVersion() const&;
		
		/** Set the http version string, must add underlying buffer first unless it's static string */
		void setVersion(std::string_view version);
		
		/** Get request headers */
		HttpRequestHeaders& getHeaders() &;
		const HttpRequestHeaders& getHeaders() const&;
		
		/** Set request header, must add underlying buffer first unless it's a static string */
		void setHeader(std::string_view key, std::string_view value);
		
		/** Set request header to integer value */
		template <class T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
		void setHeader(std::string_view key, T value);
		
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
		
		/** Get underlying buffers */
		UnderlyingBuffersType& getUnderlyingBuffers() &;
		const UnderlyingBuffersType& getUnderlyingBuffers() const&;
		
		/** Add underlying buffer that owns the storage of string views */
		std::string_view addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf);
		
		/** Constructor */
		HttpRequest();
		
		/** Constructor for null request */
		explicit HttpRequest(nullptr_t);

	private:
		Reusable<HttpRequestData> data_;
	};
	
	/** Set request header to integer value */
	template <class T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
	void HttpRequest::setHeader(std::string_view key, T value) {
		if (value >= 0 && static_cast<std::size_t>(value) < constants::Integers.size()) {
			// optimize for small integer values
			setHeader(key, constants::Integers[value]);
		} else {
			auto buf = convertIntToBuffer(value);
			setHeader(key, addUnderlyingBuffer(std::move(buf)));
		}
	}
}

