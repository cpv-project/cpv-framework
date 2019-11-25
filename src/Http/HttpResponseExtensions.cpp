#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/DateUtils.hpp>

namespace cpv::extensions {
	/** Reply 302 Found with given location to http response */
	seastar::future<> redirectTo(HttpResponse& response, SharedString&& location) {
		response.setStatusCode(constants::_302);
		response.setStatusMessage(constants::Found);
		response.setHeader(constants::Location, std::move(location));
		return seastar::make_ready_future<>();
	}

	/** Reply 301 Moved Permanently with give location to http response */
	seastar::future<> redirectToPermanently(HttpResponse& response, SharedString&& location) {
		response.setStatusCode(constants::_301);
		response.setStatusMessage(constants::MovedPermanently);
		response.setHeader(constants::Location, std::move(location));
		return seastar::make_ready_future<>();
	}

	/** Add or replace cookie on client side */
	void setCookie(
		HttpResponse& response,
		const SharedString& key,
		const SharedString& value,
		const SharedString& path,
		const SharedString& domain,
		std::optional<std::time_t> expires,
		bool httpOnly,
		bool secure,
		const SharedString& sameSite) {
		// complete field example:
		// Set-Cookie: {}={}; Path={}; Domain={}; Expires={}; HttpOnly; Secure; SameSite={}
		SharedStringBuilder builder(
			key.size() + value.size() +
			path.size() + domain.size() + sameSite.size() +
			HttpHeaderTimeStringSize + 57);
		builder.append(key);
		if (!value.empty()) {
			builder.append("=").append(value);
		}
		if (!path.empty()) {
			builder.append("; Path=").append(path);
		}
		if (!domain.empty()) {
			builder.append("; Domain=").append(domain);
		}
		if (expires.has_value()) {
			builder.append("; Expires=").append(formatTimeForHttpHeader(*expires));
		}
		if (httpOnly) {
			builder.append("; HttpOnly");
		}
		if (secure) {
			builder.append("; Secure");
		}
		if (!sameSite.empty()) {
			builder.append("; SameSite=").append(sameSite);
		}
		response.getHeaders().addAdditionHeader(constants::SetCookie, builder.build());
	}

	/** Remove cookie on client side */
	void removeCookie(
		HttpResponse& response,
		const SharedString& key,
		const SharedString& path,
		const SharedString& domain) {
		setCookie(response, key, "", path, domain, 0);
	}
}

