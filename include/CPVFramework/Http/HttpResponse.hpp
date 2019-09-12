#pragma once
#include <string_view>
#include <limits>
#include <seastar/core/temporary_buffer.hh>
#include "../Allocators/StackAllocator.hpp"
#include "../Stream/OutputStreamBase.hpp"
#include "../Utility/Reusable.hpp"
#include "../Utility/BufferUtils.hpp"
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
		using UnderlyingBuffersType = StackAllocatedVector<seastar::temporary_buffer<char>, 32>;
		
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
		HttpResponseHeaders& getHeaders() &;
		const HttpResponseHeaders& getHeaders() const&;
		
		/** Set response header, must add underlying buffer first unless it's a static string */
		void setHeader(std::string_view key, std::string_view value);
		
		/** Set response header to integer value */
		template <class T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
		void setHeader(std::string_view key, T value);
		
		/** Get response body output stream, must check whether is null before access */
		const Reusable<OutputStreamBase>& getBodyStream() const&;
		
		/** Set response body output stream */
		void setBodyStream(Reusable<OutputStreamBase>&& bodyStream);
		
		/** Get underlying buffers */
		UnderlyingBuffersType& getUnderlyingBuffers() &;
		const UnderlyingBuffersType& getUnderlyingBuffers() const&;
		
		/** Add underlying buffer that owns the storage of string views */
		void addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf);
		
		/** Constructor */
		HttpResponse();
		
		/** Constructor for null response */
		explicit HttpResponse(nullptr_t);
		
	private:
		Reusable<HttpResponseData> data_;
	};
	
	/** Set response header to integer value */
	template <class T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
	void HttpResponse::setHeader(std::string_view key, T value) {
		if (value >= 0 && static_cast<std::size_t>(value) < constants::Integers.size()) {
			setHeader(key, constants::Integers[value]);
		} else {
			auto buf = convertIntToBuffer(value);
			setHeader(key, std::string_view(buf.get(), buf.size()));
			addUnderlyingBuffer(std::move(buf));
		}
	}
}

