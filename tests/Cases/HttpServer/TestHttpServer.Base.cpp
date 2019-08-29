#include <seastar/core/future-util.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/HttpServer/HttpServer.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include "./TestHttpServer.Base.hpp"

namespace cpv::gtest {
	/** Generic test runner for http server */
	seastar::future<> runHttpServerTest(HttpServerTestFunctions&& testFunctions) {
		return seastar::do_with(
			std::move(testFunctions),
			std::make_unique<std::atomic_bool>(false),
			std::exception_ptr(),
			[] (auto& testFunctions, auto& stopFlag, auto& error) {
			return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
				[&testFunctions, &stopFlag, &error] (unsigned c) {
				return seastar::smp::submit_to(c, [c, &testFunctions, &stopFlag, &error] {
					// make configuration
					HttpServerConfiguration configuration;
					configuration.setListenAddresses({
						joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
						joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
					});
					if (testFunctions.updateConfiguration != nullptr) {
						testFunctions.updateConfiguration(configuration);
					}
					// make logger
					auto logger = Logger::createConsole(LogLevel::Debug);
					// make handlers
					HttpServerRequestHandlerCollection handlers;
					handlers.emplace_back(seastar::make_shared<HttpServerRequest500Handler>(logger));
					if (testFunctions.makeHandlers != nullptr) {
						auto customHandlers = testFunctions.makeHandlers();
						std::move(customHandlers.begin(), customHandlers.end(), std::back_inserter(handlers));
					}
					handlers.emplace_back(seastar::make_shared<HttpServerRequest404Handler>());
					// make container
					Container container;
					container.add(configuration);
					container.add(logger);
					for (auto& handler : handlers) {
						container.add(handler);
					}
					// start http server
					HttpServer server(container);
					return seastar::do_with(std::move(server),
						[c, &testFunctions, &stopFlag, &error](auto& server) {
						return server.start().then([c, &testFunctions, &stopFlag, &error] {
							// target function will run on core 0
							if (c == 0) {
								return testFunctions.execute().then([&stopFlag] {
									stopFlag->store(true);
								}).handle_exception([&stopFlag, &error] (std::exception_ptr ex) {
									error = std::move(ex);
									stopFlag->store(true);
								});
							} else {
								return seastar::do_until(
									[&stopFlag] { return stopFlag->load(); },
									[] { return seastar::sleep(std::chrono::milliseconds(50)); });
							}
						}).then([&server] {
							return server.stop();
						}).then([&error] {
							if (error == nullptr) {
								return seastar::make_ready_future<>();
							} else {
								return seastar::make_exception_future<>(std::move(error));
							}
						});
					});
				});
			});
		});
	}
	
	namespace {
		// Provide a fixed date value to make the response content always same
		const std::string_view PesudoDate("Thu, 01 Jan 1970 00:00:00 GMT");
	}
	
	/** Reply request url and header values in response body */
	seastar::future<> HttpCheckHeadersHandler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator&) const {
		using namespace cpv;
		auto& request = context.request;
		auto& response = context.response;
		Packet p;
		p.append("request method: ").append(request.getMethod()).append("\r\n")
			.append("request url: ").append(request.getUrl()).append("\r\n")
			.append("request version: ").append(request.getVersion()).append("\r\n")
			.append("request headers:\r\n");
		request.getHeaders().foreach([&p] (const auto& key, const auto& value) {
			p.append("  ").append(key).append(": ").append(value).append("\r\n");
		});
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		response.setHeader(constants::ContentLength, p.size());
		return extensions::writeAll(response.getBodyStream(), std::move(p));
	}
	
	/** Reply request body in response body */
	seastar::future<> HttpCheckBodyHandler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator&) const {
		auto& request = context.request;
		auto& response = context.response;
		return extensions::readAll(request.getBodyStream()).then([&request, &response] (auto str) {
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, request.getHeaders().getHeader(constants::ContentType));
			response.setHeader(constants::Date, PesudoDate);
			response.setHeader(constants::ContentLength, str.size());
			return extensions::writeAll(response.getBodyStream(), std::move(str));
		});
	}
	
	/** Reply response with body but without content length header */
	seastar::future<> HttpLengthNotFixedHandler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator&) const {
		auto& response = context.response;
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		return extensions::writeAll(response.getBodyStream(), "Length Not Fixed");
	}
	
	/** Reply response with body but size not matched to content length header */
	seastar::future<> HttpWrittenSizeNotMatchedHandler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator&) const {
		auto& response = context.response;
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		response.setHeader(constants::ContentLength, 1);
		return extensions::writeAll(response.getBodyStream(), "Written Size Not Matched");
	}
}

