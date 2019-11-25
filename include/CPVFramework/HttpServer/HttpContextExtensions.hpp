#pragma once
#include "../Http/HttpRequestExtensions.hpp"
#include "../Http/HttpResponseExtensions.hpp"
#include "./HttpContext.hpp"

namespace cpv::extensions {
	// namespace for getParameter(HttpContext, ...);
	// you can use `using namespace cpv::extensions::http_context_parameters;` for convenient
	namespace http_context_parameters {
		/** Index of path fragment */
		enum class PathFragment : std::size_t { };
		/** Key of query parameters */
		class Query : public SharedString { using SharedString::SharedString; };
		/** Service from container (single) */
		template <class TService> class Service { };
		/** Services from container (multiple) */
		template <class TService> class Services { };
		/** Json body and model type */
		template <class Model> class JsonModel { };
		/** Form body and model type */
		template <class Model> class FormModel { };
	}

	/** Get paramter by index of path fragment */
	static inline SharedString getParameter(
		const HttpContext& context,
		http_context_parameters::PathFragment index) {
		return context.getRequest().getUri()
			.getPathFragment(static_cast<std::size_t>(index));
	}

	/** Get paramter by key of query parameters */
	static inline SharedString getParameter(
		const HttpContext& context,
		const http_context_parameters::Query& key) {
		return context.getRequest().getUri().getQueryParameter(key);
	}

	// TODO: implement getParameter for Service
	// TODO: implement getParameter for Services
	// TODO: implement getParameter for JsonModel
	// TODO: implement getParameter for FormModel
}

