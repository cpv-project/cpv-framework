#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>

namespace cpv::extensions {
	/** Set response header from given key and unsigned integer value */
	void setHeader(HttpResponse& response, const std::string_view& key, std::size_t value) {
		if (value < constants::Integers.size()) {
			response.setHeader(key, constants::Integers[value]);
		} else {
			auto buf = convertIntToBuffer(value);
			response.setHeader(key, std::string_view(buf.get(), buf.size()));
			response.addUnderlyingBuffer(std::move(buf));
		}
	}
}

