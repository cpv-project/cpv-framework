#include <algorithm>
#include <limits>
#include <cstring>
#include <CPVFramework/Exceptions/OverflowException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>

namespace cpv {
	/** Minimal temporary buffer capacity for mergeContent */
	static const std::size_t MinTemporaryBufferCapacityForMerge = 512;
	
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
}

