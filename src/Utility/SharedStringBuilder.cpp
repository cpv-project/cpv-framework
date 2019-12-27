#include <CPVFramework/Utility/SharedStringBuilder.hpp>

namespace cpv {
	namespace {
		/** Get max string size of integer, doesn't include zero terminator */
		template <class IntType>
		constexpr std::size_t getMaxStringSize(std::size_t base) {
			IntType max = std::numeric_limits<IntType>::max();
			std::size_t result = 1; // the first digit
			if (std::numeric_limits<IntType>::is_signed) {
				++result; // the sign
			} 
			while (static_cast<std::make_unsigned_t<IntType>>(max) >= base) {
				++result;
				max /= base;
			}
			return result;
		}
		
		/** For appending integer */
		static const char DecimalDigits[] = "0123456789";

		/** Remove tailing zero for string converted from double and append to builder */
		template <class CharType>
		void trimTailingZeroAndAppend(
			BasicSharedStringBuilder<CharType>& builder, const char* data, std::size_t size) {
			const char* end = data + size;
			while (data < end && *(end - 1) == '0') {
				--end;
			}
			if (data < end && *(end - 1) == '.') {
				--end;
			} else {
				bool constantsDot = false;
				for (const char* begin = data; begin < end; ++begin) {
					constantsDot |= (*begin == '.');
				}
				if (!constantsDot) {
					end = data + size; // should not trim 12300 to 123
				}
			}
			CharType* ptr = builder.grow(end - data);
			while (data < end) {
				*ptr++ = static_cast<CharType>(*data++);
			}
		}
	}

	/** Append string representation of signed integer to end */
	template <class CharType>
	BasicSharedStringBuilder<CharType>&
		BasicSharedStringBuilder<CharType>::appendImpl(std::intmax_t value) {
		static const constexpr std::size_t MaxSize = getMaxStringSize<std::intmax_t>(10);
		CharType* ptr = grow(MaxSize);
		if (value < 0) {
			*ptr++ = '-';
		}
		CharType* ptrStart = ptr;
		do {
			int rem = value % 10;
			value /= 10;
			*ptr++ = static_cast<CharType>(DecimalDigits[rem >= 0 ? rem : -rem]);
		} while (value != 0);
		std::reverse(ptrStart, ptr);
		view_ = { buffer_.data(), static_cast<std::size_t>(ptr - buffer_.data()) };
		return *this;
	}

	/** Append string representation of unsigned integer to end */
	template <class CharType>
	BasicSharedStringBuilder<CharType>&
		BasicSharedStringBuilder<CharType>::appendImpl(std::uintmax_t value) {
		static const constexpr std::size_t MaxSize = getMaxStringSize<std::uintmax_t>(10);
		CharType* ptr = grow(MaxSize);
		CharType* ptrStart = ptr;
		do {
			unsigned int rem = value % 10;
			value /= 10;
			*ptr++ = static_cast<CharType>(DecimalDigits[rem]);
		} while (value != 0);
		std::reverse(ptrStart, ptr);
		view_ = { buffer_.data(), static_cast<std::size_t>(ptr - buffer_.data()) };
		return *this;
	}

	/** Append string representation of double to end */
	template <class CharType>
	BasicSharedStringBuilder<CharType>&
		BasicSharedStringBuilder<CharType>::appendImpl(double value) {
		// to_chars for float is not available in gcc 9, use snprintf now
		thread_local static std::array<char, 512> buffer;
		int size = std::snprintf(buffer.data(), buffer.size(), "%lf", value);
		if (CPV_LIKELY(size > 0)) {
			trimTailingZeroAndAppend(*this, buffer.data(), size);
		} else {
			std::string str = std::to_string(value);
			trimTailingZeroAndAppend(*this, str.data(), str.size());
		}
		return *this;
	}

	/** Append string representation of long double to end */
	template <class CharType>
	BasicSharedStringBuilder<CharType>&
		BasicSharedStringBuilder<CharType>::appendImpl(long double value) {
		// to_chars for float is not available in gcc 9, use snprintf now
		thread_local static std::array<char, 1024> buffer;
		int size = std::snprintf(buffer.data(), buffer.size(), "%Lf", value);
		if (CPV_LIKELY(size > 0)) {
			trimTailingZeroAndAppend(*this, buffer.data(), size);
		} else {
			std::string str = std::to_string(value);
			trimTailingZeroAndAppend(*this, str.data(), str.size());
		}
		return *this;
	}

	// Template instatiations
	template class BasicSharedStringBuilder<char>;
}

