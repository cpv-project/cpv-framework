#pragma once
#include <ctime>
#include <string_view>
#include <string>
#include <optional>
#include "./HttpResponse.hpp"

namespace cpv::extensions {
	/** Add or replace cookie on client side */
	void setCookie(
		HttpResponse& response,
		std::string_view key,
		std::string_view value,
		std::string_view path = "/",
		std::string_view domain = "",
		std::optional<std::time_t> expires = std::nullopt,
		bool httpOnly = false,
		bool secure = false,
		std::string_view sameSite = "");
	
	/** Remove cookie on client side */
	void removeCookie(
		HttpResponse& response,
		std::string_view key,
		std::string_view path = "/",
		std::string_view domain = "");
}

