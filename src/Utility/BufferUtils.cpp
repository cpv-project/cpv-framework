#include <cstring>
#include <algorithm>
#include <limits>
#include <CPVFramework/Exceptions/OverflowException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	namespace {
		/** Minimal temporary buffer capacity for mergeContent */
		static const std::size_t MinTemporaryBufferCapacityForMerge = 512;
		
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
		
		/** For convertIntToBuffer */
		static const char DecimalDigits[] = "0123456789";
	}
	
	/** Store existsContent + newContent to buffer, make existsContent point to merged content */
	void mergeContent(
		seastar::temporary_buffer<char>& buffer,
		std::string_view& existsContent,
		const std::string_view& newContent) {
		if (CPV_UNLIKELY(std::numeric_limits<std::size_t>::max() - existsContent.size() < newContent.size())) {
			throw OverflowException(CPV_CODEINFO, "size of existsContent + newContent overflowed");
		}
		std::size_t newSize = existsContent.size() + newContent.size();
		if (newSize > buffer.size()) {
			// allocate new buffer and copy both content
			// buffer size * 2 may overflow but max function will solve it
			seastar::temporary_buffer<char> newBuffer(std::max(
				std::max(newSize, buffer.size() * 2), MinTemporaryBufferCapacityForMerge));
			std::memcpy(newBuffer.get_write(), existsContent.data(), existsContent.size());
			std::memcpy(newBuffer.get_write() + existsContent.size(), newContent.data(), newContent.size());
			buffer = std::move(newBuffer);
		} else {
			// capacity enough, just append newContent
			std::memcpy(buffer.get_write() + existsContent.size(), newContent.data(), newContent.size());
		}
		// make existsContent point to merged content
		existsContent = std::string_view(buffer.get(), newSize);
	}
	
	/** Convert unsigned integer to string and return a temporary buffer store the string */
	seastar::temporary_buffer<char> convertIntToBuffer(std::uintmax_t value) {
		static const constexpr std::size_t MaxStringSize = getMaxStringSize<std::uintmax_t>(10);
		seastar::temporary_buffer<char> buf(MaxStringSize);
		char* lastChar = buf.get_write() + MaxStringSize - 1;
		do {
			std::size_t rem = value % 10;
			value /= 10;
			*lastChar = DecimalDigits[rem];
			--lastChar;
		} while (value != 0);
		buf.trim_front(lastChar - buf.get());
		return buf;
	}
	
	/** Convert signed integer to string and return a temporary buffer store the string */
	seastar::temporary_buffer<char> convertIntToBuffer(std::intmax_t value) {
		static const constexpr std::size_t MaxStringSize = getMaxStringSize<std::intmax_t>(10);
		seastar::temporary_buffer<char> buf(MaxStringSize);
		char* lastChar = buf.get_write() + MaxStringSize - 1;
		bool isNegative = value < 0;
		do {
			std::intmax_t rem = value % 10;
			value /= 10;
			*lastChar = DecimalDigits[rem >= 0 ? rem : -rem];
			--lastChar;
		} while (value != 0);
		if (isNegative) {
			*lastChar = '-';
		}
		buf.trim_front(lastChar - buf.get());
		return buf;
	}
}

