#pragma once
#include "./HttpRequest.hpp"
#include "../Serialize/FormDeserializer.hpp"
#include "../Serialize/JsonDeserializer.hpp"
#include "../Stream/InputStreamExtensions.hpp"
#include "../Utility/ObjectTrait.hpp"

namespace cpv::extensions {
	/** Read all data from request body stream and return as string */
	static inline seastar::future<SharedString>
		readBodyStream(const HttpRequest& request) {
		return readAll(request.getBodyStream());
	}

	/** Read json from request body stream and convert to model */
	template <class T>
	seastar::future<T> readBodyStreamAsJson(const HttpRequest& request) {
		return readBodyStream(request).then([] (SharedString str) {
			T model;
			auto error = deserializeJson(model, str);
			if (CPV_UNLIKELY(error.has_value())) {
				return seastar::make_exception_future<T>(*error);
			}
			return seastar::make_ready_future<T>(std::move(model));
		});
	}

	/** Read form body from request body stream and convert to model */
	template <class T>
	seastar::future<T> readBodyStreamAsForm(const HttpRequest& request) {
		return readBodyStream(request).then([] (SharedString str) {
			// TODO: support multiple part form
			T model;
			deserializeForm(model, str);
			return model;
		});
	}
}

