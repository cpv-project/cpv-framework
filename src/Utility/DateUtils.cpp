#include <array>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>

namespace cpv {
	namespace {
		// e.g. "Thu, 01 Jan 1970 00:00:00 GMT"
		static const std::size_t TimeLengthForHttpHeader = 29;
	}
	
	/** Format time for http header */
	std::size_t formatTimeForHttpHeader(::time_t time, char* buf, std::size_t size) {
		struct ::tm tmVal;
		::gmtime_r(&time, &tmVal);
		std::size_t ret = strftime(buf, size, "%a, %d %b %Y %H:%M:%S GMT", &tmVal);
		if (CPV_UNLIKELY(ret == 0)) {
			throw LengthException(CPV_CODEINFO, "buffer size not enough");
		}
		return ret;
	}
	
	/** Format time for http header, returns a thread local static string */
	std::string_view formatTimeForHttpHeader(::time_t time) {
		static thread_local ::time_t previousTime = 0;
		static thread_local std::array<char, TimeLengthForHttpHeader+1> buf(([] {
			// avoid empty result if the first call pass 0 as time
			std::array<char, TimeLengthForHttpHeader+1> buf;
			formatTimeForHttpHeader(0, buf.data(), buf.size());
			return buf;
		})());
		if (CPV_UNLIKELY(time != previousTime)) {
			previousTime = time;
			formatTimeForHttpHeader(time, buf.data(), buf.size());
		}
		return std::string_view(buf.data(), TimeLengthForHttpHeader);
	}
	
	/** Format now for http header, returns a thread local static string */
	std::string_view formatNowForHttpHeader() {
		return formatTimeForHttpHeader(::time(nullptr));
	}
}
