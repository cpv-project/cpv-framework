#pragma once
#include <tuple>
#include "../../Http/HttpRequestExtensions.hpp"
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Custom checker that checks whether Func invocable with given parameters */
	template <class Params, class Func, std::size_t... I>
	constexpr bool HttpServerRequestParametersFunctionHandlerTypeChecker(std::index_sequence<I...>) {
		return std::is_invocable_r_v<seastar::future<>, Func, HttpContext&, decltype(
			extensions::getParameter(
				std::declval<HttpRequest>(),
				std::get<I>(std::declval<Params>())))...>;
	}

	/**
	 * Request handler that use custom function object and invoke it with given parameters.
	 * Params should be a tuple contains types that supports cpv::extensions::getParameter
	 * for HttpRequest, like integer represents the index of path fragment, and string
	 * represents the key of query parameter.
	 * For example:
	 * when params = std::make_tuple(1, "abc"), this handler will invoke func with
	 * func(context, request.getUri().getPathFragment(1), request.getUri().getQueryParameter("abc"));
	 */
	template <class Params, class Func,
		std::enable_if_t<
			HttpServerRequestParametersFunctionHandlerTypeChecker<Params, Func>(
				std::make_index_sequence<std::tuple_size_v<Params>>()), int> = 0>
	class HttpServerRequestParametersFunctionHandler : public HttpServerRequestHandlerBase {
	public:
		/** Invoke custom function object with given parameters */
		seastar::future<> handle(
			HttpContext& context,
			const HttpServerRequestHandlerIterator&) const override {
			return handleImpl(context,
				std::make_index_sequence<std::tuple_size_v<Params>>());
		}

		/** Constructor */
		HttpServerRequestParametersFunctionHandler(Params params, Func func) :
			params_(std::move(params)), func_(std::move(func)) { }

	private:
		/** The implementation of handle */
		template <std::size_t... I>
		seastar::future<> handleImpl(
			HttpContext& context, std::index_sequence<I...>) const {
			auto& request = context.getRequest();
			return func_(context,
				extensions::getParameter(request, std::get<I>(params_))...);
		}

	private:
		Params params_;
		Func func_;
	};
}

