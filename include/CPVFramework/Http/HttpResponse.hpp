#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <seastar/core/temporary_buffer.hh>
#include "../Stream/OutputStream.hpp"
#include "../Utility/Object.hpp"

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
		std::string_view getVersion() const&;
		
		/** Set the http version string, must add underlying buffer first unless it's static string */
		void setVersion(std::string_view version);
		
		/** Get the status code, e.g. "404" */
		std::string_view getStatusCode() const&;
		
		/** Set the status code, must add underlying buffer first unless it's static string */
		void setStatusCode(std::string_view statusCode);
		
		/** Get the reason message of status code, e.g. "Not Found" */
		std::string_view getStatusMessage() const&;
		
		/**
		 * Set the reason message of status code,
		 *	must add underlying buffer first unless it's static string.
		 */
		void setStatusMessage(std::string_view statusMessage);
		
		/** Get response headers */
		std::unordered_map<std::string_view, std::string_view>& getHeaders() &;
		const std::unordered_map<std::string_view, std::string_view>& getHeaders() const&;
		
		/** Set response header, must add underlying buffer first unless it's static string */
		void setHeader(std::string_view key, std::string_view value);
		
		/** Add underlying buffer that owns the storage of string views */
		void addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf);
		
		/** Get response body output stream */
		Object<OutputStream>& getBodyStream() &;
		
		/** Set response body output stream */
		void setBodyStream(Object<OutputStream>&& bodyStream);
		
		/** Constructor */
		HttpResponse();
		
	private:
		Object<HttpResponseData> data_;
	};
}

