#pragma once
#include "./HttpRequest.hpp"
#include "../Stream/InputStreamExtensions.hpp"

namespace cpv::extensions {
	/** Read all data from request body stream and return as string */
	static inline seastar::future<std::string>
		readBodyStream(const HttpRequest& request) {
		return readAll(request.getBodyStream());
	}

	/** Read all data from request body stream and return as buffer */
	static inline seastar::future<seastar::temporary_buffer<char>>
		readBodyStreamAsBuffer(const HttpRequest& request) {
		return readAllAsBuffer(request.getBodyStream());
	}

	/** Read all data from request body stream and parse as json then save to model */
	template <class T>
	seastar::future<> readBodyStreamAsJson(const HttpRequest& request, T& model) {
		throw 1;
	}

	/** Read all data from request body stream and parse as json then save to model */
	template <class T>
	seastar::future<> readBodyStreamAsForm(const HttpRequest& request, T& model) {
		throw 1;
	}

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

