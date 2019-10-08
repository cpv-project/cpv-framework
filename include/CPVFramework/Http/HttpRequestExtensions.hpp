#pragma once
#include "./HttpRequest.hpp"

namespace cpv::extensions {
	/** Get parameter from request path fragments */
	static inline std::string_view getParameter(
		const HttpRequest& request, std::size_t index) {
		return request.getUri().getPathFragment(index);
	}

	/** Get parameter from request query paramters */
	static inline std::string_view getParameter(
		const HttpRequest& request, std::string_view key) {
		return request.getUri().getQueryParameter(key);
	}
}

