#pragma once
#include "./HttpRequest.hpp"
#include "../Stream/InputStreamExtensions.hpp"

namespace cpv::extensions {
	/** Read all data from request body stream and return as string */
	static inline seastar::future<SharedString>
		readBodyStream(const HttpRequest& request) {
		return readAll(request.getBodyStream());
	}

	/** Read all data from request body stream and parse as json then save to model */
	template <class T>
	seastar::future<> readBodyStreamAsJson(const HttpRequest& request, T& model) {
		// TODO
		throw 1;
	}

	/** Read all data from request body stream and parse as json then save to model */
	template <class T>
	seastar::future<> readBodyStreamAsForm(const HttpRequest& request, T& model) {
		// TODO
		throw 1;
	}
}

