#pragma once
#include "./HttpResponse.hpp"

namespace cpv::extensions {
	/** Set response header from given key and unsigned integer value */
	void setHeader(HttpResponse& response, const std::string_view& key, std::size_t value);
}

