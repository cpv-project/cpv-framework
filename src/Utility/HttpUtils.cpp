#include <cstring>
#include <CPVFramework/Utility/HttpUtils.hpp>
#include <CPVFramework/Utility/Macros.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include "./HttpUtils.UrlEncodeMapping.hpp"
#include "./HttpUtils.HtmlEncodeMapping.hpp"
#include "./HttpUtils.HtmlEntitiesMapping.hpp"
#include "./HttpUtils.MimeMapping.hpp"

namespace cpv {
	/** Encode string for use in url */
	SharedString urlEncode(SharedString&& str) {
		std::size_t encodedSize = 0;
		for (char c : str) {
			encodedSize += UrlEncodeMapping[static_cast<unsigned char>(c)].size();
		}
		if (encodedSize == str.size()) {
			// no change, return original string
			// notice space will not convert to +, so we can sure by checking encoded size
			return std::move(str);
		}
		SharedString buf(encodedSize);
		char* dst = buf.data();
		for (char c : str) {
			auto mapped = UrlEncodeMapping[static_cast<unsigned char>(c)];
			if (CPV_LIKELY(mapped.size() == 1)) {
				*dst++ = c;
			} else {
				std::memcpy(dst, mapped.data(), mapped.size());
				dst += mapped.size();
			}
		}
		return buf;
	}

	/** Decode string from url parts */
	SharedString urlDecode(SharedString&& str) {
		bool changed = false;
		for (char c : str) {
			if (c == '%' || c == '+') {
				changed = true;
				break;
			}
		}
		if (!changed) {
			// no change, return original string
			return std::move(str);
		}
		SharedString buf(str.size());
		char* dst = buf.data();
		const char* src = str.begin();
		const char* srcEnd = str.end();
		std::uint8_t charFromHex = 0;
		for (; src < srcEnd; ++src) {
			const char c = *src;
			if (c == '%') {
				if (src + 2 < srcEnd) {
					loadIntFromHex(src + 1, charFromHex);
					*dst++ = static_cast<char>(charFromHex);
				}
				// ignore invalid tail '%' or '%x'
				src += 2;
			} else if (c == '+') {
				// replace + with space for compatibility
				*dst++ = ' ';
			} else {
				*dst++ = c;
			}
		}
		buf.trim(dst - buf.data());
		return buf;
	}

	/** Encode string for use in html */
	SharedString htmlEncode(SharedString&& str) {
		std::size_t encodedSize = 0;
		for (char c : str) {
			encodedSize += HtmlEncodeMapping[static_cast<unsigned char>(c)].size();
		}
		if (encodedSize == str.size()) {
			// no change, return original string
			return std::move(str);
		}
		SharedString buf(encodedSize);
		char* dst = buf.data();
		for (char c : str) {
			auto mapped = HtmlEncodeMapping[static_cast<unsigned char>(c)];
			if (CPV_LIKELY(mapped.size() == 1)) {
				*dst++ = c;
			} else {
				std::memcpy(dst, mapped.data(), mapped.size());
				dst += mapped.size();
			}
		}
		return buf;
	}

	/** Decode string from html content */
	SharedString htmlDecode(SharedString&& str) {
		bool changed = false;
		for (char c : str) {
			if (c == '&') {
				changed = true;
				break;
			}
		}
		if (!changed) {
			// no change, return original string
			return std::move(str);
		}
		SharedString buf(str.size());
		char* dst = buf.data();
		const char* src = str.begin();
		const char* srcEnd = str.end();
		const char* srcLast = str.end() - 1; // empty str will hit no change
		for (; src < srcEnd; ++src) {
			const char c = *src;
			if (c != '&') {
				*dst++ = c;
				continue;
			}
			// find name of html entity: e.g. "&quot;"
			// after for loop src should point to ';' or last character
			const char* nameBegin = src;
			for (; *src != ';' && src < srcLast; ++src) { }
			const char* nameEnd = src;
			// check whether it's unicode code point: e.g. "&#xabcd;" or "&#1;"
			if (nameBegin + 3 <= nameEnd && nameBegin[1] == '#') {
				std::size_t code = 0;
				if (nameBegin[2] == 'x' || nameBegin[2] == 'X') {
					if (!loadIntFromHex(nameBegin + 3, nameEnd - nameBegin - 3, code)) {
						continue; // ignore invalid code point
					}
				} else if (!loadIntFromDec(nameBegin + 2, nameEnd - nameBegin - 2, code)) {
					continue; // ignore invalid code point
				}
				// convert to utf-8, see: https://en.wikipedia.org/wiki/UTF-8
				if (code <= 0x7f) {
					*dst++ = static_cast<char>(code);
				} else if (code <= 0x7ff) {
					dst[0] = static_cast<char>(0b11000000 | (code >> 6));
					dst[1] = static_cast<char>(0b10000000 | (code & 0b00111111));
					dst += 2;
				} else if (code <= 0xffff) {
					dst[0] = static_cast<char>(0b11100000 | (code >> 12));
					dst[1] = static_cast<char>(0b10000000 | ((code >> 6) & 0b00111111));
					dst[2] = static_cast<char>(0b10000000 | (code & 0b00111111));
					dst += 3;
				} else if (code <= 0x10ffff) {
					dst[0] = static_cast<char>(0b11110000 | (code >> 18));
					dst[1] = static_cast<char>(0b10000000 | ((code >> 12) & 0b00111111));
					dst[2] = static_cast<char>(0b10000000 | ((code >> 6) & 0b00111111));
					dst[3] = static_cast<char>(0b10000000 | (code & 0b00111111));
					dst += 4;
				}
				// ignore invalid code point
			} else {
				// find entity from named mapping
				auto it = HtmlEntitiesMapping.find(std::string_view(
					nameBegin, nameEnd - nameBegin + 1));
				if (it != HtmlEntitiesMapping.end()) {
					auto mapped = it->second;
					std::memcpy(dst, mapped.data(), mapped.size());
					dst += mapped.size();
				}
				// ignore unsupported entity
			}
		}
		buf.trim(dst - buf.data());
		return buf;
	}

	/** Get mime type of file path (path can be extension only) */
	SharedString getMimeType(std::string_view path) {
		auto dotPos = path.find_last_of('.');
		std::string_view extension = (dotPos == path.npos) ? path : path.substr(dotPos + 1);
		auto it = MimeMapping.find(extension);
		if (it != MimeMapping.end()) {
			return SharedString::fromStatic(it->second);
		}
		return "application/octet-stream";
	}
}

