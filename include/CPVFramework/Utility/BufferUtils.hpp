#pragma once
#include <cstdint>
#include <string_view>
#include <seastar/core/temporary_buffer.hh>

namespace cpv {
	/**
	 * Store existsContent + newContent to buffer, make existsContent point to merged content.
	 * Cases:
	 * - buffer is empty, allocate new buffer and copy both content
	 * - buffer is not empty, assume it contains existsContent
	 *   - if capacity enough, just append newContent
	 *   - if capacity not enough, allocate new buffer and copy both content
	 */
	void mergeContent(
		seastar::temporary_buffer<char>& buffer,
		std::string_view& existsContent,
		const std::string_view& newContent);
	
	/** Convert unsigned integer to string and return a temporary buffer store the string */
	seastar::temporary_buffer<char> convertIntToBuffer(std::uintmax_t value);
	
	/** Convert signed integer to string and return a temporary buffer store the string */
	seastar::temporary_buffer<char> convertIntToBuffer(std::intmax_t value);
	
	/** Convert integer to string and return a temporary buffer store the string */
	template <class IntType>
	seastar::temporary_buffer<char> convertIntToBuffer(IntType value) {
		if constexpr (std::numeric_limits<IntType>::is_signed) {
			return convertIntToBuffer(static_cast<std::intmax_t>(value));
		}
		return convertIntToBuffer(static_cast<std::uintmax_t>(value));
	}
}

