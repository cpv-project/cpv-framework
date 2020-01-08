#pragma once
#include <ctime>
#include <optional>
#include "../Stream/OutputStreamExtensions.hpp"
#include "../Utility/StringUtils.hpp"
#include "../Utility/SharedString.hpp"
#include "./HttpResponse.hpp"

namespace cpv::extensions {
	/** Reply text or binary contents to http response in once */
	template <class T>
	seastar::future<> reply(
		HttpResponse& response,
		T&& text,
		SharedString&& mimeType,
		SharedString&& statusCode,
		SharedString&& statusMessage) {
		auto& headers = response.getHeaders();
		response.setStatusCode(std::move(statusCode));
		response.setStatusMessage(std::move(statusMessage));
		headers.setContentType(std::move(mimeType));
		headers.setContentLength(SharedString::fromInt(sizeofString(text)));
		return writeAll(response.getBodyStream(), std::forward<T>(text));
	}

	/** Reply text or binary contents to http response in once */
	template <class T>
	seastar::future<> reply(HttpResponse& response, T&& text, SharedString&& mimeType) {
		return reply(response, std::forward<T>(text),
			std::move(mimeType), constants::_200, constants::OK);
	}

	/** Reply text or binary contents to http response in once */
	template <class T>
	seastar::future<> reply(HttpResponse& response, T&& text) {
		return reply(response, std::forward<T>(text), constants::TextPlainUtf8);
	}

	/** Reply 302 Found with given location to http response */
	seastar::future<> redirectTo(HttpResponse& response, SharedString&& location);

	/** Reply 301 Moved Permanently with give location to http response */
	seastar::future<> redirectToPermanently(HttpResponse& response, SharedString&& location);

	/** Add or replace cookie on client side */
	void setCookie(
		HttpResponse& response,
		const SharedString& key,
		const SharedString& value,
		const SharedString& path = "/",
		const SharedString& domain = "",
		std::optional<std::time_t> expires = std::nullopt,
		bool httpOnly = false,
		bool secure = false,
		const SharedString& sameSite = "");

	/** Remove cookie on client side */
	void removeCookie(
		HttpResponse& response,
		const SharedString& key,
		const SharedString& path = "/",
		const SharedString& domain = "");
}

