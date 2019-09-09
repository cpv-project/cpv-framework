#pragma once
#include <cctype>
#include <vector>
#include <memory>
#include <sstream>
#include <type_traits>
#include <string_view>
#include <algorithm>
#include "./Macros.hpp"

namespace cpv {
	/**
	 * Split string with specified characters.
	 * Call func(startIndex, endIndex, count) while split.
	 * Default split characters are empty characters.
	 */
	template <class Func>
	static void splitString(
		const std::string& str, const Func& func, const char* chars = " \t\r\n") {
		std::size_t startIndex = 0;
		std::size_t count = 0;
		while (startIndex < str.size()) {
			auto index = str.find_first_of(chars, startIndex);
			auto endIndex = (index == str.npos) ? str.size() : index;
			func(startIndex, endIndex, count);
			index = str.find_first_not_of(chars, endIndex);
			startIndex = (index == str.npos) ? str.size() : index;
			++count;
		}
	}

	/**
	 * Join arguments into string.
	 * This function is very slow, don't call it where performance is important.
	 */
	template <class Delimiter, class... Args>
	static std::string joinString(Delimiter&& delimiter, Args&&... args) {
		std::ostringstream stream;
		bool isFirst = true;
		auto func = [&stream, &isFirst, &delimiter](auto&& arg) {
			if (isFirst) {
				isFirst = false;
			} else {
				stream << delimiter;
			}
			stream << arg;
			return 0;
		};
		int dummy[sizeof...(Args)] = { func(std::forward<Args>(args))... };
		return stream.str();
	}

	/**
	 * Convert integer to hex and write to string.
	 * IntType can be any of int??_t and uint??_t.
	 * Also see stackoverflow 5100718.
	 */
	template <class IntType, class StringType>
	void dumpIntToHex(IntType value, StringType& str) {
		static const char digits[] = "0123456789ABCDEF";
		static const std::size_t hexLen = sizeof(IntType) * 2;
		for (std::size_t i = 0, j = (hexLen-1)*4; i < hexLen; ++i, j-=4) {
			str.append(digits + ((value>>j)&0xf), 1);
		}
	}

	/**
	 * Convert integer to decimal and write to string.
	 * IntType can be any of int??_t and uint??_t.
	 */
	template <class IntType, class StringType>
	void dumpIntToDec(IntType value, StringType& str) {
		static const char digits[] = "0123456789";
		if constexpr (std::numeric_limits<IntType>::is_signed) {
			if (value < 0) {
				str.append("-");
			}
		}
		std::size_t prevSize = str.size();
		do {
			IntType rem = value % 10;
			value /= 10;
			str.append(1, digits[rem >= 0 ? rem : -rem]);
		} while (value != 0);
		std::reverse(str.begin() + prevSize, str.end());
	}

	/** Convert bytes to hex and write to string */
	template <class StringType>
	void dumpBytesToHex(const char* bytes, std::size_t length, StringType& str) {
		const char* start = bytes;
		const char* end = bytes + length;
		while (start < end) {
			std::uint8_t byte = static_cast<std::uint8_t>(*start++);
			dumpIntToHex(byte, str);
		}
	}

	/**
	 * Convert dynamic size hex string to integer.
	 * Return whether the coversion is successful.
	 */
	template <class IntType>
	bool loadIntFromHex(const char* hex, std::size_t hexLen, IntType& value) {
		using UnsignedIntType = std::make_unsigned_t<IntType>;
		std::uint8_t diff = 0;
		UnsignedIntType uvalue = 0; // avoid sanitizer warning: left shift of negative value
		const char* hexEnd = hex + hexLen;
		for (; hex < hexEnd; ++hex) {
			std::uint8_t c = static_cast<std::uint8_t>(*hex);
			if (c == 0) {
				return false;
			} else if ((diff = c - '0') <= 9) {
				uvalue = (uvalue << 4) | diff;
			} else if ((diff = c - 'a') <= 5) {
				uvalue = (uvalue << 4) | (diff + 0xa);
			} else if ((diff = c - 'A') <= 5) {
				uvalue = (uvalue << 4) | (diff + 0xa);
			} else {
				return false;
			}
		}
		value = static_cast<IntType>(uvalue);
		return true;
	}

	/**
	 * Convert fixed size (sizeof(IntType)*2) hex string to integer.
	 * Return whether the coversion is successful.
	 */
	template <class IntType>
	bool loadIntFromHex(const char* hex, IntType& value) {
		return loadIntFromHex<IntType>(hex, sizeof(IntType)*2, value);
	}

	/**
	 * Convert decimal string to integer.
	 * Return whether the coversion is successful.
	 * Notice convert minus decimal string to unsigned integer is an error,
	 * but overflow isn't an error (the overhead of overflow detection is high).
	 */
	template <class IntType>
	bool loadIntFromDec(const char* dec, std::size_t decLength, IntType& value) {
		const char* decEnd = dec + decLength;
		bool minus = false;
		if (CPV_UNLIKELY(dec < decEnd && *dec == '-')) {
			minus = true;
			++dec;
		}
		std::uint8_t diff = 0;
		value = 0;
		while (CPV_LIKELY(dec < decEnd)) {
			if (CPV_LIKELY((diff = *dec - '0') <= 9)) {
				value *= 10;
				value += diff;
				++dec;
			} else {
				return false;
			}
		}
		if constexpr (!std::numeric_limits<IntType>::is_signed) {
			if (minus) {
				return false;
			}
		} else if (minus) {
			value = -value;
		}
		return true;
	}

	/**
	 * Convert hex string to bytes.
	 * Return whether the coversion is successful.
	 */
	template <class StringType>
	bool loadBytesFromHex(const char* hex, std::size_t hexLength, StringType& str) {
		const char* start = hex;
		const char* end = hex + hexLength;
		char c = 0;
		while (start < end && *start != 0) {
			if (!loadIntFromHex<char>(start, c)) {
				return false;
			}
			str.append(&c, 1);
			start += 2;
		}
		return start == end;
	}

	/** Compare two string case insensitive */
	bool caseInsensitiveEquals(std::string_view a, std::string_view b);
}

