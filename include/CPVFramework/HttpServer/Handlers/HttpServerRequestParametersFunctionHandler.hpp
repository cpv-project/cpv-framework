#pragma once
#include <tuple>
#include "../HttpContextExtensions.hpp"
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Custom checker that checks whether Func invocable with given parameters */
	template <class Params, class Func, std::size_t... I>
	constexpr bool HttpServerRequestParametersFunctionHandlerTypeChecker(std::index_sequence<I...>) {
		return std::is_invocable_r_v<seastar::future<>, Func, HttpContext&, decltype(
			extensions::getParameter(
				std::declval<HttpContext>(),
				std::get<I>(std::declval<Params>())))...>;
	}

	/**
	 * Request handler that use custom function object and invoke it with given parameters.
	 *
	 * Params should be a tuple contains types which supports
	 * cpv::extensions::getParameter(const HttpContext&, const T&),
	 * there are some built-in types under namespace cpv::extensions::http_context_parameters,
	 * you can see them inside HttpContextExtensions.hpp.
	 *
	 * For example:
	 * ```
	 * using namespace cpv::extensions::http_context_parameters;
	 * HttpServerRequestParametersFunctionHandler handler(
	 *     std::make_tuple(PathFragment(1), Query("abc")),
	 *     [] (HttpContext& context, SharedString id, SharedString name) {
	 *         ...
	 *     });
	 * // will invoke func with:
	 * // func(context, request.getUri().getPathFragment(1), request.getUri().getQueryParameter("abc"));
	 * handler.handle(context, ...);
	 * ```
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
			HttpServerRequestHandlerIterator) const override {
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
			return func_(context,
				extensions::getParameter(context, std::get<I>(params_))...);
		}

	private:
		Params params_;
		Func func_;
	};
}

