#pragma once
#include "../Allocators/StackAllocator.hpp"
#include "../Utility/SharedString.hpp"

namespace cpv {
	/**
	 * Cookies collection for http request
	 * Notice:
	 * It will assume cookies are already encoded by url encoding or base64.
	 */
	class HttpRequestCookies {
	public:
		using CookiesType = StackAllocatedMap<SharedString, SharedString, 3>;
		
		/** Get cookie value for given key, return empty string if key not exists */
		SharedString get(const SharedString& key) const;
		
		/** Get all cookies */
		const CookiesType& getAll() const&;
		
		/** Parse the value from Cookie header */
		void parse(const SharedString& cookies);
		
		/** Clear all parsed cookies */
		void clear();
		
	private:
		/** Constructor */
		HttpRequestCookies();
		
		// make auto generated constructors and assign operators private
		HttpRequestCookies(const HttpRequestCookies&) = default;
		HttpRequestCookies(HttpRequestCookies&&) = default;
		HttpRequestCookies& operator=(const HttpRequestCookies&) = default;
		HttpRequestCookies& operator=(HttpRequestCookies&&) = default;
		
		friend class HttpRequestData;
		
	private:
		CookiesType cookies_;
	};
}

