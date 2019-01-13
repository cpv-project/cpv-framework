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
			bodyStream(makeObject<StringInputStream>(std::string()).cast<InputStreamBase>()) { }
		
		void freeResources() {
			method = {};
			url = {};
			version = {};
			headers.clear();
			bodyStream = makeObject<StringInputStream>(std::string()).cast<InputStreamBase>();
			underlyingBuffers.clear();
		}
		
		static void reset() { }
	};
}

