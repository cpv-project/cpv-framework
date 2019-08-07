#pragma once
#include <string_view>
#include <unordered_map>
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Http/HttpResponse.hpp>

namespace cpv {
	/** Members of HttpResponse */
	class HttpResponseData {
	public:
		HttpResponse::UnderlyingBuffersType underlyingBuffers;
		std::string_view version;
		std::string_view statusCode;
		std::string_view statusMessage;
		HttpResponseHeaders headers;
		Reusable<OutputStreamBase> bodyStream;
		
		HttpResponseData() :
			underlyingBuffers(),
			version(),
			statusCode(),
			statusMessage(),
			headers(),
			bodyStream() { }
		
		void freeResources() {
			version = {};
			statusCode = {};
			statusMessage = {};
			headers = {};
			bodyStream = Reusable<OutputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
	
	/** Increase free list size */
	template <>
	const constexpr std::size_t ReusableStorageCapacity<HttpResponseData> = 28232;
}

