#include <CPVFramework/Http/HttpRequestCookies.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>

namespace cpv {
	/** Get cookie value for given key, return empty string if key not exists */
	std::string_view HttpRequestCookies::get(std::string_view key) const {
		auto it = cookies_.find(key);
		return it != cookies_.end() ? it->second : std::string_view();
	}
	
	/** Get all cookies */
	const HttpRequestCookies::CookiesType& HttpRequestCookies::getAll() const& {
		return cookies_;
	}
	
	/** Parse the value from Cookie header */
	void HttpRequestCookies::parse(std::string_view cookies) {
		// examples:
		// key
		// key=value
		// key=value; other-key=other-value; other-key-only
		const char* mark = cookies.begin();
		const char* ptr = mark;
		const char* end = cookies.end();
		std::string_view key;
		std::string_view value;
		for (; ptr < end; ++ptr) {
			const char c = *ptr;
			if (c == '=') {
				key = trimString(std::string_view(mark, ptr - mark));
				mark = ptr + 1;
			} else if (c == ';') {
				value = trimString(std::string_view(mark, ptr - mark));
				mark = ptr + 1;
				if (!key.empty()) {
					cookies_.insert_or_assign(key, value);
				} else if (!value.empty()) {
					cookies_.insert_or_assign(value, "");
				}
				key = {};
				value = {};
			}
		}
		if (mark < ptr) {
			value = trimString(std::string_view(mark, ptr - mark));
			if (!key.empty()) {
				cookies_.insert_or_assign(key, value);
			} else if (!value.empty()) {
				cookies_.insert_or_assign(value, "");
			}
		}
	}
	
	/** Clear all parsed cookies */
	void HttpRequestCookies::clear() {
		cookies_.clear();
	}
	
	/** Constructor */
	HttpRequestCookies::HttpRequestCookies() :
		cookies_() { }
}

