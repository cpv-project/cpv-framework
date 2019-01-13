#pragma once
#include <string_view>
#include <unordered_map>
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>

namespace cpv {
	/** Members of HttpRequest */
	class HttpRequestData {
	public:
		std::vector<seastar::temporary_buffer<char>> underlyingBuffers;
		std::string_view method;
		std::string_view url;
		std::string_view version;
		std::unordered_map<std::string_view, std::string_view> headers;
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
			headers.clear();
			bodyStream = Object<InputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
}

