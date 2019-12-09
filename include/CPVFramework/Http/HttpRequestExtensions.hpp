#pragma once
#include "./HttpRequest.hpp"
#include "../Serialize/FormDeserializer.hpp"
#include "../Serialize/JsonDeserializer.hpp"
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
		return readBodyStream(request).then([&model] (SharedString str) {
			auto error = deserializeJson(model, str);
			if (CPV_UNLIKELY(error.has_value())) {
				return seastar::make_exception_future<>(*error);
			}
			return seastar::make_ready_future<>();
		});
	}

	/** Read all data from request body stream and parse as json then save to model */
	template <class T>
	seastar::future<> readBodyStreamAsForm(const HttpRequest& request, T& model) {
		return readBodyStream(request).then([&model] (SharedString str) {
			deserializeForm(model, str);
		});
	}
}

