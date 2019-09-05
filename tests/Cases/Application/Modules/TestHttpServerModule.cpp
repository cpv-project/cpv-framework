#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/Exceptions/Exception.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	class Test500Handler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator& next) const override {
			return seastar::futurize_apply([&context, &next] {
				return (*next)->handle(context, next + 1);
			}).handle_exception([&context] (std::exception_ptr) {
				auto& response = context.getResponse();
				response.setStatusCode(cpv::constants::_500);
				response.setStatusMessage("Custom Internal Server Error");
				return seastar::make_ready_future<>();
			});
		}
	};
}

TEST_FUTURE(TestHttpServerModule, startStop) {
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
	return application.start().then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(TestHttpServerModule, default404Handler) {
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
	return application.start()
	.then([] {
		cpv::Packet p(
			"GET /not_found HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n");
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_CONTAINS(str, "HTTP/1.1 404 Not Found\r\n");
		});
	 }).then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(TestHttpServerModule, default500Handler) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createNoop());
	});
	application.add<cpv::HttpServerModule>([] (auto& module) {
		module.getConfig().setListenAddresses({
			cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
			cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
		});
		module.addCustomHandler([] (cpv::HttpContext&) -> seastar::future<> {
			throw cpv::Exception(CPV_CODEINFO, "test exception");
		});
	});
	return application.start()
	.then([] {
		cpv::Packet p(
			"GET / HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n");
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_CONTAINS(str, "HTTP/1.1 500 Internal Server Error\r\n");
		});
	 }).then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(TestHttpServerModule, set404Handler) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createNoop());
	});
	application.add<cpv::HttpServerModule>([] (auto& module) {
		module.getConfig().setListenAddresses({
			cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
			cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
		});
		module.set404Handler([] (cpv::HttpContext& context) {
			auto& response = context.getResponse();
			response.setStatusCode(cpv::constants::_404);
			response.setStatusMessage("Custom Not Found");
			return seastar::make_ready_future<>();
		});
	});
	return application.start()
	.then([] {
		cpv::Packet p(
			"GET /not_found HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n");
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_CONTAINS(str, "HTTP/1.1 404 Custom Not Found\r\n");
		});
	 }).then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(TestHttpServerModule, set500Handler) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createNoop());
	});
	application.add<cpv::HttpServerModule>([] (auto& module) {
		module.getConfig().setListenAddresses({
			cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
			cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
		});
		module.set500Handler(seastar::make_shared<Test500Handler>());
		module.addCustomHandler([] (cpv::HttpContext&) -> seastar::future<> {
			throw cpv::Exception(CPV_CODEINFO, "test exception");
		});
	});
	return application.start()
	.then([] {
		cpv::Packet p(
			"GET / HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n");
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_CONTAINS(str, "HTTP/1.1 500 Custom Internal Server Error\r\n");
		});
	 }).then([application] () mutable {
		return application.stop();
	});
}

TEST_FUTURE(TestHttpServerModule, addCustomHandler) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createNoop());
	});
	application.add<cpv::HttpServerModule>([] (auto& module) {
		module.getConfig().setListenAddresses({
			cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
			cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
		});
		module.addCustomHandler([] (cpv::HttpContext& context) {
			auto& response = context.getResponse();
			response.setStatusCode(cpv::constants::_200);
			response.setStatusMessage("Custom OK");
			return seastar::make_ready_future<>();
		});
	});
	return application.start()
	.then([] {
		cpv::Packet p(
			"GET / HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n");
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_CONTAINS(str, "HTTP/1.1 200 Custom OK\r\n");
		});
	 }).then([application] () mutable {
		return application.stop();
	});
}

