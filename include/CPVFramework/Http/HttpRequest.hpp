#pragma once
#include <string_view>
#include <unordered_map>
#include <seastar/core/temporary_buffer.hh>
#include "../Stream/InputStreamBase.hpp"
#include "../Utility/Object.hpp"

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
		std::string_view getMethod() const&;
		
		/* Set the request method, must add underlying buffer first unless it's static string */
		void setMethod(const std::string_view& method);
		
		/** Get the request url, e.g. "/test" */
		std::string_view getUrl() const&;
		
		/** Set the request url, must add underlying buffer first unless it's static string */
		void setUrl(const std::string_view& url);
		
		/** Get the http version string, e.g. "HTTP/1.1" */
		std::string_view getVersion() const&;
		
		/** Set the http version string, must add underlying buffer first unless it's static string */
		void setVersion(const std::string_view& version);
		
		/** Get request headers */
		std::unordered_map<std::string_view, std::string_view>& getHeaders() &;
		const std::unordered_map<std::string_view, std::string_view>& getHeaders() const&;
		
		/** Set request header, must add underlying buffer first unless it's static string */
		void setHeader(const std::string_view& key, const std::string_view& value);
		
		/** Get underlying buffers */
		std::vector<seastar::temporary_buffer<char>>& getUnderlyingBuffers() &;
		const std::vector<seastar::temporary_buffer<char>>& getUnderlyingBuffers() const&;
		
		/**
		 * Add underlying buffer that owns the storage of string views,
		 *	return a string view of the buffer.
		 */
		std::string_view addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf);
		
		/** Get request body input stream, must check whether is null before access */
		const Object<InputStreamBase>& getBodyStream() const&;
		
		/** Set request body input stream */
		void setBodyStream(Object<InputStreamBase>&& bodyStream);
		
		/** Constructor */
		HttpRequest();

	private:
		Object<HttpRequestData> data_;
	};
}

