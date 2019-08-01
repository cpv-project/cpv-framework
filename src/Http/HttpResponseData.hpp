#pragma once
#include <string_view>
#include <unordered_map>
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>
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
		Object<OutputStreamBase> bodyStream;
		
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
			bodyStream = Object<OutputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
	
	/** Increase free list size */
	template <>
	struct ObjectFreeListSize<HttpResponseData> {
		static const constexpr std::size_t value = 65535;
	};
}

