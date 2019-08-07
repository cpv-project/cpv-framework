#pragma once
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
		HttpRequest::UnderlyingBuffersType underlyingBuffers;
		std::string_view method;
		std::string_view url;
		std::string_view version;
		HttpRequestHeaders headers;
		Reusable<InputStreamBase> bodyStream;
		
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
			bodyStream = Reusable<InputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
}

