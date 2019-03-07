#include <algorithm>
#include <atomic>
#include <seastar/core/future-util.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/HttpServer/HttpServer.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/PacketUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	using HandlersType = std::vector<std::unique_ptr<cpv::HttpServerRequestHandlerBase>>;
	
	seastar::future<> runTest(
		std::function<HandlersType()> handlersFactory,
		std::function<seastar::future<>()> func) {
		return seastar::do_with(
			std::move(handlersFactory),
			std::move(func),
			std::make_unique<std::atomic_bool>(false),
			[] (auto& handlersFactory, auto& func, auto& stopFlag) {
			return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
				[&handlersFactory, &func, &stopFlag] (unsigned c) {
				return seastar::smp::submit_to(c, [c, &handlersFactory, &func, &stopFlag] {
					cpv::HttpServerConfiguration configuration;
					configuration.setListenAddresses({
						cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
						cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
					});
					auto logger = cpv::Logger::createConsole(cpv::LogLevel::Debug);
					HandlersType customHandlers(handlersFactory());
					HandlersType handlers;
					handlers.emplace_back(std::make_unique<cpv::HttpServerRequest500Handler>(logger));
					std::move(customHandlers.begin(), customHandlers.end(), std::back_inserter(handlers));
					handlers.emplace_back(std::make_unique<cpv::HttpServerRequest404Handler>());
					cpv::HttpServer server(configuration, logger, std::move(handlers));
					return seastar::do_with(std::move(server), [c, &func, &stopFlag](auto& server) {
						return server.start().then([c, &func, &stopFlag] {
							// func will run on core 0
							if (c == 0) {
								return func().then([&stopFlag] { stopFlag->store(true); });
							} else {
								return seastar::do_until(
									[&stopFlag] { return stopFlag->load(); },
									[] { return seastar::sleep(std::chrono::milliseconds(50)); });
							}
						}).then([&server] {
							return server.stop();
						});
					});
				});
			});
		});
	}
	
	class CheckHeadersHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpRequest& request,
			cpv::HttpResponse& response,
			const cpv::HttpServerRequestHandlerIterator&) const override {
			using namespace cpv;
			seastar::net::packet p;
			p << "request method: " << request.getMethod() << "\r\n";
			p << "request url: " << request.getUrl() << "\r\n";
			p << "request version: " << request.getVersion() << "\r\n";
			p << "request headers:\r\n";
			auto& headers = request.getHeaders();
			std::vector<std::string_view> headerFields;
			std::transform(headers.begin(), headers.end(),
				std::back_inserter(headerFields), [] (auto& pair) { return pair.first; });
			std::sort(headerFields.begin(), headerFields.end());
			for (auto& headerField : headerFields) {
				p << "  " << headerField << ": " << headers.at(headerField) << "\r\n";
			}
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			response.setHeader(constants::Date, "Thu, 01 Jan 1970 00:00:00 GMT");
			if (p.len() < constants::Integers.size()) {
				response.setHeader(constants::ContentLength, constants::Integers.at(p.len()));
			} else {
				auto buf = convertIntToBuffer(p.len());
				response.setHeader(constants::ContentLength, std::string_view(buf.get(), buf.size()));
				response.addUnderlyingBuffer(std::move(buf));
			}
			return extensions::writeAll(response.getBodyStream(), std::move(p));
		}
	};
}

TEST_FUTURE(HttpServer_Http11, headers) {
	return runTest([] {
		std::vector<std::unique_ptr<cpv::HttpServerRequestHandlerBase>> handlers;
		handlers.emplace_back(std::make_unique<CheckHeadersHandler>());
		return handlers;
	}, [] {
		using namespace cpv;
		seastar::net::packet p;
		p << "GET /test_headers HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Connection: close\r\n"
			"User-Agent: TestClient\r\n\r\n";
		return cpv::gtest::tcpSendRequest(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT, std::move(p))
		.then([] (std::string str) {
			ASSERT_EQ(str,
				"HTTP/1.1 200 OK\r\n"
				"Date: Thu, 01 Jan 1970 00:00:00 GMT\r\n"
				"Server: cpv-framework\r\n"
				"Connection: close\r\n"
				"Content-Length: 160\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n\r\n"
				"request method: GET\r\n"
				"request url: /test_headers\r\n"
				"request version: HTTP/1.1\r\n"
				"request headers:\r\n"
				"  Connection: close\r\n"
				"  Host: localhost\r\n"
				"  User-Agent: TestClient\r\n");
		});
	});
}

