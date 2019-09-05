#include <seastar/core/future-util.hh>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(HttpServerRequest404Handler, handle) {
	return seastar::do_with(
		cpv::HttpServerRequestHandlerCollection(),
		cpv::HttpContext(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& handlers, auto& context, auto& str) {
		context.getResponse().setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest404Handler>());
		return handlers.at(0)->handle(context, handlers.end()).then([&context, &str] {
			auto& response = context.getResponse();
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_404);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::NotFound);
			auto& headers = response.getHeaders();
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), cpv::constants::TextPlainUtf8);
			ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength),
				cpv::constants::Integers.at(cpv::constants::NotFound.size()));
			ASSERT_EQ(*str, cpv::constants::NotFound);
		});
	});
}

