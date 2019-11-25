#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerRoutingModule.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestHttpServerRoutingModule, route) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createNoop());
	});
	application.add<cpv::HttpServerModule>([] (auto& module) {
		module.getConfig().setListenAddresses({
			cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
			cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
		});
	});
	application.add<cpv::HttpServerRoutingModule>([] (auto& module) {
		using namespace cpv::extensions::http_context_parameters;
		module.route(cpv::constants::GET, "/", [] (auto& context) {
			return cpv::extensions::reply(context.getResponse(), "index page");
		});
		module.route(cpv::constants::GET, "/get/*",
			std::make_tuple(PathFragment(1), Query("page"), Query("sort")),
			[] (auto& context, auto category, auto page, auto sort) {
				cpv::Packet p;
				p.append(std::move(category)).append("-")
					.append(std::move(page)).append("-")
					.append(std::move(sort));
				return cpv::extensions::reply(context.getResponse(), std::move(p));
			});
	});
	return application.start()
	.then([] {
		cpv::Packet p(
			"GET / HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"User-Agent: TestClient\r\n\r\n"
			"GET /get/oranges?sort=price&page=1 HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n"
			);
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_CONTAINS(str, "index page");
			ASSERT_CONTAINS(str, "oranges-1-price");
		});
	 }).then([application] () mutable {
		return application.stop();
	});
}

