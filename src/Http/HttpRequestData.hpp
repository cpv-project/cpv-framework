#pragma once
#include <string_view>
#include <unordered_map>
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>
#include <CPVFramework/Http/HttpRequest.hpp>

namespace cpv {
	/** Members of HttpRequest */
	class HttpRequestData {
	public:
		HttpRequest::UnderlyingBuffersType underlyingBuffers;
		std::string_view method;
		std::string_view url;
		std::string_view version;
		HttpRequestHeaders headers;
		Object<InputStreamBase> bodyStream;
		
		HttpRequestData() :
			underlyingBuffers(),
			method(),
			version(),
			headers(),
			bodyStream() { }
		
		void freeResources() {
			method = {};
			url = {};
			version = {};
			headers = {};
			bodyStream = Object<InputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
	
	/** Increase free list size */
	template <>
	struct ObjectFreeListSize<HttpRequestData> {
		static const constexpr std::size_t value = 65535;
	};
}

