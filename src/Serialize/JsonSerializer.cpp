#include <CPVFramework/Serialize/JsonSerializer.hpp>
#include "./JsonSerializer.JsonEncodeMapping.hpp"

namespace cpv {
	/** Encode string for use in json */
	SharedString jsonEncode(SharedString&& str) {
		std::size_t encodedSize = 0;
		for (char c : str) {
			encodedSize += JsonEncodeMapping[static_cast<unsigned char>(c)].size();
		}
		if (encodedSize == str.size()) {
			// no change, return original string
			return std::move(str);
		}
		SharedString buf(encodedSize);
		char* dst = buf.data();
		for (char c : str) {
			auto mapped = JsonEncodeMapping[static_cast<unsigned char>(c)];
			if (CPV_LIKELY(mapped.size() == 1)) {
				*dst++ = c;
			} else {
				std::memcpy(dst, mapped.data(), mapped.size());
				dst += mapped.size();
			}
		}
		return buf;
	}
}

