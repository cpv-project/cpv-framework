#include <CPVFramework/Http/HttpRequestCookies.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>

namespace cpv {
	/** Get cookie value for given key, return empty string if key not exists */
	SharedString HttpRequestCookies::get(const SharedString& key) const {
		auto it = cookies_.find(key);
		return it != cookies_.end() ? it->second.share() : SharedString();
	}
	
	/** Get all cookies */
	const HttpRequestCookies::CookiesType& HttpRequestCookies::getAll() const& {
		return cookies_;
	}
	
	/** Parse the value from Cookie header */
	void HttpRequestCookies::parse(const SharedString& cookies) {
		// examples:
		// key
		// key=value
		// key=value; other-key=other-value; other-key-only
		const char* mark = cookies.begin();
		const char* ptr = mark;
		const char* end = cookies.end();
		SharedString key;
		SharedString value;
		for (; ptr < end; ++ptr) {
			const char c = *ptr;
			if (c == '=') {
				key = cookies.share(trimString(
					{ mark, static_cast<std::size_t>(ptr - mark) }));
				mark = ptr + 1;
			} else if (c == ';') {
				value = cookies.share(trimString(
					{ mark, static_cast<std::size_t>(ptr - mark) }));
				mark = ptr + 1;
				if (!key.empty()) {
					cookies_.insert_or_assign(std::move(key), std::move(value));
				} else if (!value.empty()) {
					cookies_.insert_or_assign(std::move(value), "");
				}
				key = {};
				value = {};
			}
		}
		if (mark < ptr) {
			value = cookies.share(trimString(
				{ mark, static_cast<std::size_t>(ptr - mark) }));
			if (!key.empty()) {
				cookies_.insert_or_assign(std::move(key), std::move(value));
			} else if (!value.empty()) {
				cookies_.insert_or_assign(std::move(value), "");
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

