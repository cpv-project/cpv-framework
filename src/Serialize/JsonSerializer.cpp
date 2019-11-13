#include <CPVFramework/Serialize/JsonSerializer.hpp>
#include "./JsonSerializer.JsonEncodeMapping.hpp"

namespace cpv {
	/** Encode string for use in json */
	std::pair<std::string_view, seastar::temporary_buffer<char>> jsonEncode(std::string_view str) {
		std::size_t encodedSize = 0;
		for (char c : str) {
			encodedSize += JsonEncodeMapping[static_cast<unsigned char>(c)].size();
		}
		if (encodedSize == str.size()) {
			// no change, return original string
			return std::make_pair(str, seastar::temporary_buffer<char>());
		}
		seastar::temporary_buffer<char> buf(encodedSize);
		char* dst = buf.get_write();
		for (char c : str) {
			auto mapped = JsonEncodeMapping[static_cast<unsigned char>(c)];
			if (CPV_LIKELY(mapped.size() == 1)) {
				*dst++ = c;
			} else {
				std::memcpy(dst, mapped.data(), mapped.size());
				dst += mapped.size();
			}
		}
		return std::make_pair(std::string_view(buf.get(), buf.size()), std::move(buf));
	}
}

