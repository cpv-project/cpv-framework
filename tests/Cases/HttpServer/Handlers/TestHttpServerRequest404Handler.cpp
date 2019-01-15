#include <seastar/core/future-util.hh>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(HttpServerRequest404Handler, handle) {
	return seastar::do_with(
		std::vector<std::unique_ptr<cpv::HttpServerRequestHandlerBase>>(),
		cpv::HttpRequest(),
		cpv::HttpResponse(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& handlers, auto& request, auto& response, auto& str) {
		handlers.emplace_back(std::make_unique<cpv::HttpServerRequest404Handler>());
		response.setBodyStream(
			cpv::makeObject<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());
		return handlers.at(0)->handle(request, response, handlers.end()).then([&response, &str] {
			ASSERT_EQ(response.getStatusCode(), cpv::constants::_404);
			ASSERT_EQ(response.getStatusMessage(), cpv::constants::NotFound);
			ASSERT_EQ(response.getHeaders().at(cpv::constants::ContentType),
				cpv::constants::TextPlainUtf8);
			ASSERT_EQ(response.getHeaders().at(cpv::constants::ContentLength),
				cpv::constants::Integers.at(cpv::constants::NotFound.size()));
			ASSERT_EQ(*str, cpv::constants::NotFound);
		});
	});
}

