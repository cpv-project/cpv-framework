#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/core/future.hh>
#include <CPVFramework/Utility/Object.hpp>

namespace cpv {
	/** Members of HttpServerRequest */
	class HttpServerRequestData;
	
	/**
	 * Contains headers, body and addition informations of a http request.
	 * It should contains only data so it can be mock easily.
	 */
	class HttpServerRequest {
	public:
		/** Get the request method, e.g. "GET" */
		std::string_view getMethod() const&;
		
		/** Set the request method */
		void setMethod(std::string&& method);
		
		/* Set the request method, must add underlying buffer first unless it's static string */
		void setMethod(std::string_view method);
		
		/** Get the request url, e.g. "/test" */
		std::string_view getUrl() const&;
		
		/** Set the request url */
		void setUrl(std::string&& url);
		
		/** Set the request url, must add underlying buffer first unless it's static string */
		void setUrl(std::string_view url);
		
		/** Get the http version string, e.g. "HTTP/1.1" */
		std::string_view getVersion() const&;
		
		/** Set the http version string */
		void setVersion(std::string&& version);
		
		/** Set the http version string, must add underlying buffer first unless it's static string */
		void setVersion(std::string_view version);
		
		/** Get request headers */
		std::unordered_map<std::string_view, std::string_view>& getHeaders() &;
		const std::unordered_map<std::string_view, std::string_view>& getHeaders() const&;
		
		/** Set request header */
		void setHeader(std::string&& key, std::string&& value);
		
		/** Set request header, must add underlying buffer first unless it's static string */
		void setHeader(std::string_view key, std::string_view value);
		
		/**
		 * Get request body.
		 * May trigger http connection reading so it's an async operation.
		 */
		seastar::future<std::string_view> getBody() const&;
		
		/** Set request body */
		void setBody(std::string&& body);
		
		/** Set request body, must add underlying buffer first unless it's static string */
		void setBody(std::string_view body);
		
		/** Set request body, must take care about the lifetime and always set result or exception */
		void setBody(seastar::future<std::string_view>&& body);
		
		/** Add underlying buffer that owns the storage of string views */
		void addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf);
		void addUnderlyingBuffer(std::string&& buf);
		
		/** Constructor */
		HttpServerRequest();

	private:
		Object<HttpServerRequestData> data_;
	};
}

