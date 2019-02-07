#pragma once
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
}

