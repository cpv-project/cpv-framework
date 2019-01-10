#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/core/future.hh>
#include <CPVFramework/Utility/Object.hpp>

namespace cpv {
	/** Members of HttpServerResponse */
	class HttpServerResponseData;
	
	/**
	 * Contains headers, body and addition informations of a http response.
	 * It should contains only data so it can be mock easily.
	 */
	class HttpServerResponse {
	public:
		/** Get the http version string, e.g. "HTTP/1.1" */
		std::string_view getVersion() const&;
		
		/** Set the http version string */
		void setVersion(std::string&& version);
		
		/** Set the http version string, must add underlying buffer first unless it's static string */
		void setVersion(std::string_view version);
		
		/** Get the status code, e.g. "404" */
		std::string_view getStatusCode() const&;
		
		/** Set the status code */
		void setStatusCode(std::string&& statusCode);
		
		/** Set the status code, must add underlying buffer first unless it's static string */
		void setStatusCode(std::string_view statusCode);
		
		/** Get the reason message of status code, e.g. "Not Found" */
		std::string_view getStatusMessage() const&;
		
		/** Set the reason message of status code */
		void setStatusMessage(std::string&& statusMessage);
		
		/**
		 * Set the reason message of status code,
		 *	must add underlying buffer first unless it's static string.
		 */
		void setStatusMessage(std::string_view statusMessage);
		
		/** Get response headers */
		std::unordered_map<std::string_view, std::string_view>& getHeaders() &;
		const std::unordered_map<std::string_view, std::string_view>& getHeaders() const&;
		
		/** Set response header */
		void setHeader(std::string&& key, std::string&& value);
		
		/** Set response header, must add underlying buffer first unless it's static string */
		void setHeader(std::string_view key, std::string_view value);
		
		/** Get response body */
		std::string_view getBody() const&;
		
		/** Set response body */
		void setBody(std::string&& body);
		
		/** Set response body, must add underlying buffer first unless it's static string */
		void setBody(std::string_view body);
		
		/**
		 * Set response body,
		 *	the appender may called multiple times,
		 *	it must return an empty string to indicate there no more string to append.
		 */
		void setBody(std::function<std::string_view()> bodyAppender);
		
		/** Add underlying buffer that owns the storage of string views */
		void addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf);
		void addUnderlyingBuffer(std::string&& buf);
		
		/** Constructor */
		HttpServerResponse();
		
	private:
		Object<HttpServerResponseData> data_;
	};
}

