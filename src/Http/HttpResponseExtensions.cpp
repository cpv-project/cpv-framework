#include <seastar/core/temporary_buffer.hh>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/DateUtils.hpp>

namespace cpv::extensions {
	/** Reply 302 Found with given location to http response */
	seastar::future<> redirectTo(HttpResponse& response, std::string_view location) {
		response.setStatusCode(constants::_302);
		response.setStatusMessage(constants::Found);
		response.setHeader(constants::Location, location);
		return seastar::make_ready_future<>();
	}

	/** Reply 301 Moved Permanently with give location to http response */
	seastar::future<> redirectToPermanently(HttpResponse& response, std::string_view location) {
		response.setStatusCode(constants::_301);
		response.setStatusMessage(constants::MovedPermanently);
		response.setHeader(constants::Location, location);
		return seastar::make_ready_future<>();
	}

	/** Add or replace cookie on client side */
	void setCookie(
		HttpResponse& response,
		std::string_view key,
		std::string_view value,
		std::string_view path,
		std::string_view domain,
		std::optional<std::time_t> expires,
		bool httpOnly,
		bool secure,
		std::string_view sameSite) {
		// complete field example:
		// Set-Cookie: {}={}; Path={}; Domain={}; Expires={}; HttpOnly; Secure; SameSite={}
		seastar::temporary_buffer<char> buf(
			key.size() + value.size() +
			path.size() + domain.size() + sameSite.size() +
			HttpHeaderTimeStringSize + 57);
		std::string_view view;
		mergeContent(buf, view, key);
		if (!value.empty()) {
			mergeContent(buf, view, "=");
			mergeContent(buf, view, value);
		}
		if (!path.empty()) {
			mergeContent(buf, view, "; Path=");
			mergeContent(buf, view, path);
		}
		if (!domain.empty()) {
			mergeContent(buf, view, "; Domain=");
			mergeContent(buf, view, domain);
		}
		if (expires.has_value()) {
			HttpHeaderTimeStringBufferType buffer;
			formatTimeForHttpHeader(*expires, buffer.data(), buffer.size());
			mergeContent(buf, view, "; Expires=");
			mergeContent(buf, view, std::string_view(buffer.data(), buffer.size()-1));
		}
		if (httpOnly) {
			mergeContent(buf, view, "; HttpOnly");
		}
		if (secure) {
			mergeContent(buf, view, "; Secure");
		}
		if (!sameSite.empty()) {
			mergeContent(buf, view, "; SameSite=");
			mergeContent(buf, view, sameSite);
		}
		response.addUnderlyingBuffer(std::move(buf));
		response.getHeaders().addAdditionHeader(constants::SetCookie, view);
	}

	/** Remove cookie on client side */
	void removeCookie(
		HttpResponse& response,
		std::string_view key,
		std::string_view path,
		std::string_view domain) {
		setCookie(response, key, "", path, domain, 0);
	}
}

