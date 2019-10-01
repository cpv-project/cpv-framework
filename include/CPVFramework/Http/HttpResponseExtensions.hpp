#pragma once
#include <ctime>
#include <string_view>
#include <string>
#include <optional>
#include "../Stream/OutputStreamExtensions.hpp"
#include "../Utility/StringUtils.hpp"
#include "./HttpResponse.hpp"

namespace cpv::extensions {
	/** Reply text or binary contents to http response in once */
	template <class T>
	seastar::future<> reply(
		HttpResponse& response,
		T&& text,
		std::string_view mimeType,
		std::string_view statusCode,
		std::string_view statusMessage) {
		response.setStatusCode(statusCode);
		response.setStatusMessage(statusMessage);
		response.setHeader(constants::ContentType, mimeType);
		response.setHeader(constants::ContentLength, sizeofString(text));
		return writeAll(response.getBodyStream(), std::forward<T>(text));
	}

	/** Reply text or binary contents to http response in once */
	template <class T>
	seastar::future<> reply(HttpResponse& response, T&& text, std::string_view mimeType) {
		return reply(response, std::forward<T>(text), mimeType, constants::_200, constants::OK);
	}

	/** Reply text or binary contents to http response in once */
	template <class T>
	seastar::future<> reply(HttpResponse& response, T&& text) {
		return reply(response, std::forward<T>(text), constants::TextPlainUtf8);
	}

	/** Reply 302 Found with given location to http response */
	seastar::future<> redirectTo(HttpResponse& response, std::string_view location);

	/** Reply 301 Moved Permanently with give location to http response */
	seastar::future<> redirectToPermanently(HttpResponse& response, std::string_view location);

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

