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
		std::string_view method;
		std::string_view url;
		std::string_view version;
		HttpRequestHeaders headers;
		Reusable<InputStreamBase> bodyStream;
		// mutable for lazy parse
		mutable std::string_view sourceOfUri;
		mutable HttpRequestUri uri;
		mutable std::string_view sourceOfCookies;
		mutable HttpRequestCookies cookies;
		HttpRequest::UnderlyingBuffersType underlyingBuffers;
		
		HttpRequestData() :
			method(),
			version(),
			headers(),
			bodyStream(),
			uri(),
			underlyingBuffers() { }
		
		void freeResources() {
			method = {};
			url = {};
			version = {};
			headers.clear();
			bodyStream = Reusable<InputStreamBase>();
			sourceOfUri = {};
			uri.clear();
			sourceOfCookies = {};
			cookies.clear();
			underlyingBuffers.clear();
		}
		
		inline void ensureUriUpdated() const {
			if (sourceOfUri.empty()) {
				sourceOfUri = url;
				uri.parse(url);
			} else if (sourceOfUri.data() != url.data()) {
				// compare pointer directly
				sourceOfUri = url;
				uri.clear();
				uri.parse(url);
			}
		}
		
		inline void ensureCookiesUpdated() const {
			auto cookieHeader = headers.getCookie();
			if (sourceOfCookies.empty()) {
				sourceOfCookies = cookieHeader;
				cookies.parse(cookieHeader);
			} else if (sourceOfCookies.data() != cookieHeader.data()) {
				// compare pointer directly
				sourceOfCookies = cookieHeader;
				cookies.clear();
				cookies.parse(cookieHeader);
			}
		}
		
		static void reset() { }
	};

	/** Increase free list size */
	template <>
	const constexpr std::size_t ReusableStorageCapacity<HttpRequestData> = 28232;
}

