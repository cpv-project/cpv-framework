#pragma once
#include <ctime>
#include <string>
#include <array>

namespace cpv {
	/** The size of time string used for http header, not include tailing zero */
	static const constexpr std::size_t HttpHeaderTimeStringSize = 29;
	
	/** The buffer type used to format time string for http header */
	using HttpHeaderTimeStringBufferType = std::array<char, HttpHeaderTimeStringSize+1>;
	
	/**
	 * Format time for http header.
	 * e.g. "Thu, 31 May 2007 20:35:00 GMT"
	 * The buffer size should >= HttpHeaderTimeStringSize + 1,
	 * it will return true for success and false for buffer size not enough.
	 */
	bool formatTimeForHttpHeader(std::time_t time, char* buf, std::size_t size);
	
	/** Format time for http header, returns a thread local static string */
	std::string_view formatTimeForHttpHeader(std::time_t time);
	
	/**
	 * Format now for http header, returns a thread local static string
	 * Notice:
	 * The thread local storage of this function is distinct (not shared with
	 * the other overload), so it can be safe to used directly for Date header.
	 */
	std::string_view formatNowForHttpHeader();
}

