#pragma once
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Request handler that use custom function object */
	template <class Func,
		std::enable_if_t<std::is_invocable_r_v<
			seastar::future<>, Func, HttpContext&>, int> = 0>
	class HttpServerRequestFunctionHandler : public HttpServerRequestHandlerBase {
	public:
		/** Invoke custom function object */
		seastar::future<> handle(
			HttpContext& context,
			const HttpServerRequestHandlerIterator&) const override {
			return func_(context);
		}

		/** Constructor */
		HttpServerRequestFunctionHandler(Func func) :
			func_(std::move(func)) { }

	private:
		Func func_;
	};
}

