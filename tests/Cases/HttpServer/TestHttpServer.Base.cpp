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
#include "TestHttpServer.Base.hpp"

namespace cpv::gtest {
	/** Generic test runner for http server */
	seastar::future<> runHttpServerTest(HttpServerTestFunctions&& testFunctions) {
		return seastar::do_with(
			std::move(testFunctions),
			std::make_unique<std::atomic_bool>(false),
			[] (auto& testFunctions, auto& stopFlag) {
			return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
				[&testFunctions, &stopFlag] (unsigned c) {
				return seastar::smp::submit_to(c, [c, &testFunctions, &stopFlag] {
					// make configuration
					cpv::HttpServerConfiguration configuration;
					configuration.setListenAddresses({
						cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
						cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
					});
					if (testFunctions.updateConfiguration != nullptr) {
						testFunctions.updateConfiguration(configuration);
					}
					// make logger
					auto logger = cpv::Logger::createConsole(cpv::LogLevel::Debug);
					// make handlers
					HttpServerRequestHandlerCollection handlers;
					handlers.emplace_back(std::make_unique<cpv::HttpServerRequest500Handler>(logger));
					if (testFunctions.makeHandlers != nullptr) {
						auto customHandlers = testFunctions.makeHandlers();
						std::move(customHandlers.begin(), customHandlers.end(), std::back_inserter(handlers));
					}
					handlers.emplace_back(std::make_unique<cpv::HttpServerRequest404Handler>());
					// start http server
					cpv::HttpServer server(configuration, logger, std::move(handlers));
					return seastar::do_with(std::move(server), [c, &testFunctions, &stopFlag](auto& server) {
						return server.start().then([c, &testFunctions, &stopFlag] {
							// target function will run on core 0
							if (c == 0) {
								return testFunctions.execute().then([&stopFlag] {
									stopFlag->store(true);
								});
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
	
	/** Reply url and header values in response body */
	seastar::future<> HttpCheckHeadersHandler::handle(
		cpv::HttpRequest& request,
		cpv::HttpResponse& response,
		const cpv::HttpServerRequestHandlerIterator&) const {
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
}

