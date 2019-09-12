#include <array>
#include <optional>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>

namespace cpv {
	/** Format time for http header */
	bool formatTimeForHttpHeader(std::time_t time, char* buf, std::size_t size) {
		struct std::tm tmVal;
		::gmtime_r(&time, &tmVal);
		std::size_t ret = std::strftime(buf, size, "%a, %d %b %Y %H:%M:%S GMT", &tmVal);
		return ret != 0;
	}
	
	/** Format time for http header, returns a thread local static string */
	std::string_view formatTimeForHttpHeader(std::time_t time) {
		static thread_local std::optional<std::time_t> previousTime;
		static thread_local HttpHeaderTimeStringBufferType buf;
		if (CPV_UNLIKELY(!previousTime.has_value() || time != previousTime.value())) {
			previousTime = time;
			formatTimeForHttpHeader(time, buf.data(), buf.size());
		}
		return std::string_view(buf.data(), HttpHeaderTimeStringSize);
	}
	
	/** Format now for http header, returns a thread local static string */
	std::string_view formatNowForHttpHeader() {
		return formatTimeForHttpHeader(std::time(nullptr));
	}
}

