#pragma once
#include <string_view>
#include <utility>
#include <seastar/core/temporary_buffer.hh>

namespace cpv {
	/**
	 * Encode string for use in url,
	 * may return original string and empty buffer if no change.
	 * Notice: space will not replace to '+'.
	 */
	std::pair<std::string_view, seastar::temporary_buffer<char>> urlEncode(std::string_view str);

	/**
	 * Decode string from url parts,
	 * may return original string and empty buffer if no change.
	 * Notice:
	 * it will ignore format errors.
	 * '+' will replace to space for compatibility.
	 * unicode characters such as "%u4e00" are unsupported.
	 */
	std::pair<std::string_view, seastar::temporary_buffer<char>> urlDecode(std::string_view str);

	/**
	 * Encode string for use in html,
	 * may return original string and empty buffer if no change.
	 * Notice:
	 * characters except '<', '>', '&', '"', '\'' are keep as is,
	 * please use "Content-Type: text/html; charset=utf-8" to support unicode characters.
	 */
	std::pair<std::string_view, seastar::temporary_buffer<char>> htmlEncode(std::string_view str);

	/**
	 * Decode string from html content,
	 * may return original string and empty buffer if no change.
	 * Notice:
	 * it will ignore format errors.
	 * unicode characters such as "&#x4e00;" and "&#120171;" are supported.
	 */
	std::pair<std::string_view, seastar::temporary_buffer<char>> htmlDecode(std::string_view str);

	/** Get mime type of file path (path can be extension only) */
	std::string_view getMimeType(std::string_view path);
}

