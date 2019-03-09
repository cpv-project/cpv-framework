#pragma once
#include "./HttpRequest.hpp"

namespace cpv::extensions {
	/** Set request header from given key and unsigned integer value */
	void setHeader(HttpRequest& request, const std::string_view& key, std::size_t value);
}

