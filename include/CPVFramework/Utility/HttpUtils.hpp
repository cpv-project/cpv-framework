#pragma once
#include <utility>
#include "./SharedString.hpp"

namespace cpv {
	/**
	 * Encode string for use in url,
	 * may return original string if not changed.
	 * Notice: space will not replace to '+'.
	 */
	SharedString urlEncode(SharedString&& str);

	/**
	 * Decode string from url parts,
	 * may return original string if not changed.
	 * Notice:
	 * it will ignore format errors.
	 * '+' will replace to space for compatibility.
	 * unicode characters such as "%u4e00" are unsupported.
	 */
	SharedString urlDecode(SharedString&& str);

	/**
	 * Encode string for use in html,
	 * may return original string if not changed.
	 * Notice:
	 * characters except '<', '>', '&', '"', '\'' are keep as is,
	 * please use "Content-Type: text/html; charset=utf-8" to support unicode characters.
	 */
	SharedString htmlEncode(SharedString&& str);

	/**
	 * Decode string from html content,
	 * may return original string if not changed.
	 * Notice:
	 * it will ignore format errors.
	 * unicode characters such as "&#x4e00;" and "&#120171;" are supported.
	 */
	SharedString htmlDecode(SharedString&& str);

	/** Get mime type of file path (path can be extension only) */
	SharedString getMimeType(std::string_view path);
}

