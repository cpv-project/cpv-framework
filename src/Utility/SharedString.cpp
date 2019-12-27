#include <string>
#include <CPVFramework/Utility/SharedString.hpp>
#include <CPVFramework/Utility/SharedStringBuilder.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>

namespace cpv {
	/** Convert string to signed interger value, overflow is not checked */
	template <class CharType>
	std::optional<std::intmax_t> BasicSharedString<CharType>::toIntImpl() const {
		static_assert(sizeof(CharType) == 1, "size of char type must be 1");
		std::intmax_t value;
		if (CPV_LIKELY(loadIntFromDec(
			reinterpret_cast<const char*>(data()), size(), value))) {
			return value;
		}
		return std::nullopt;
	}

	/** Convert string to unsigned interger value, overflow is not checked */
	template <class CharType>
	std::optional<std::uintmax_t> BasicSharedString<CharType>::toUintImpl() const {
		static_assert(sizeof(CharType) == 1, "size of char type must be 1");
		std::uintmax_t value;
		if (CPV_LIKELY(loadIntFromDec(
			reinterpret_cast<const char*>(data()), size(), value))) {
			return value;
		}
		return std::nullopt;
	}

	/** Convert string to double value */
	template <class CharType>
	std::optional<double> BasicSharedString<CharType>::toDoubleImpl() const {
		static_assert(sizeof(CharType) == 1, "size of char type must be 1");
		// from_chars for float is not available in gcc 9, use strtod now
		static thread_local std::string buffer; // ensure null terminated
		buffer.assign(reinterpret_cast<const char*>(data()), size());
		char* strEnd;
		double value = std::strtod(buffer.c_str(), &strEnd);
		if (static_cast<std::size_t>(strEnd - buffer.data()) == buffer.size()) {
			return value;
		}
		return std::nullopt;
	}

	/** Convert string to long double value */
	template <class CharType>
	std::optional<long double> BasicSharedString<CharType>::toLongDoubleImpl() const {
		static_assert(sizeof(CharType) == 1, "size of char type must be 1");
		// from_chars for float is not available in gcc 9, use strtold now
		static thread_local std::string buffer; // ensure null terminated
		buffer.assign(reinterpret_cast<const char*>(data()), size());
		char* strEnd;
		long double value = std::strtold(buffer.c_str(), &strEnd);
		if (static_cast<std::size_t>(strEnd - buffer.data()) == buffer.size()) {
			return value;
		}
		return std::nullopt;
	}

	/** Construct with string representation of integer */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromIntImpl(std::intmax_t value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	/** Construct with string representation of integer */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromIntImpl(std::uintmax_t value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	/** Construct with string representation of floating point */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromDoubleImpl(double value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	/** Construct with string representation of floating point */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromDoubleImpl(long double value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	// Template instatiations
	template class BasicSharedString<char>;
}

