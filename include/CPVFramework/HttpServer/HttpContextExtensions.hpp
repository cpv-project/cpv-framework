#pragma once
#include "../Http/HttpRequestExtensions.hpp"
#include "../Http/HttpResponseExtensions.hpp"
#include "../Utility/ObjectTrait.hpp"
#include "./HttpContext.hpp"

namespace cpv::extensions {
	// namespace for getParameter(HttpContext, ...);
	// you can use `using namespace cpv::extensions::http_context_parameters;` for convenient
	namespace http_context_parameters {
		/** Index of path fragment */
		enum class PathFragment : std::size_t { };
		/** Key of query parameters */
		class Query : public SharedString { using SharedString::SharedString; };
		/** Service from container (you can use collection type to retrive multiple) */
		template <class TService> class Service { };
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

	/** Get service or services from container */
	template <class T, class = void /** for enable_if */>
	static inline T getParameter(
		const HttpContext& context,
		http_context_parameters::Service<T>) {
		if constexpr (ServiceTypeTrait<T>::IsCollection) {
			T collection; // optional object will be empty
			context.getManyServices<T>(collection);
			return collection;
		} else {
			return context.getService<T>();
		}
	}

	/** Read json from request body stream and convert to model */
	template <class T, class = void /** for enable_if */>
	static inline seastar::future<T> getParameter(
		const HttpContext& context,
		http_context_parameters::JsonModel<T>) {
		return readBodyStreamAsJson<T>(context.getRequest());
	}

	/** Read form body from request body stream and convert to model */
	template <class T, class = void /** for enable_if */>
	static inline seastar::future<T> getParameter(
		const HttpContext& context,
		http_context_parameters::FormModel<T>) {
		return readBodyStreamAsForm<T>(context.getRequest());
	}
}

