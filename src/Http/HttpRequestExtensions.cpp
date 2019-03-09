#include <CPVFramework/Http/HttpRequestExtensions.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>

namespace cpv::extensions {
	/** Set request header from given key and unsigned integer value */
	void setHeader(HttpRequest& request, const std::string_view& key, std::size_t value) {
		if (value < constants::Integers.size()) {
			request.setHeader(key, constants::Integers[value]);
		} else {
			auto buf = convertIntToBuffer(value);
			request.setHeader(key, std::string_view(buf.get(), buf.size()));
			request.addUnderlyingBuffer(std::move(buf));
		}
	}
}

