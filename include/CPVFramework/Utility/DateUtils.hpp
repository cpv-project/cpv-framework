#pragma once
#include <time.h>
#include <string>

namespace cpv {
	/**
	 * Format time for http header.
	 * e.g. "Thu, 31 May 2007 20:35:00 GMT"
	 * The buffer size should greater or equal than 30.
	 * Return the size written to buffer, should always return 29.
	 */
	std::size_t formatTimeForHttpHeader(::time_t time, char* buf, std::size_t size);
	
	/** Format time for http header, returns a thread local static string */
	std::string_view formatTimeForHttpHeader(::time_t time);
	
	/** Format now for http header, returns a thread local static string */
	std::string_view formatNowForHttpHeader();
}

