#pragma once
#include <string_view>
#include <unordered_map>
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>

namespace cpv {
	/** Members of HttpResponse */
	class HttpResponseData {
	public:
		std::vector<seastar::temporary_buffer<char>> underlyingBuffers;
		std::string_view version;
		std::string_view statusCode;
		std::string_view statusMessage;
		std::unordered_map<std::string_view, std::string_view> headers;
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
			headers.clear();
			bodyStream = Object<OutputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
}

